#ifndef YETANOTHERREGEX_HPP
#define YETANOTHERREGEX_HPP

#include <set>
#include <map>
#include <list>
#include <tuple>
#include <limits>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace yare
{
namespace details
{
constexpr char32_t kChar32Min = std::numeric_limits<char32_t>::min();
constexpr char32_t kChar32Max = std::numeric_limits<char32_t>::max();

inline std::u32string
str_to_utf8(const std::string &str)
{
    std::u32string result;
    auto reading = reinterpret_cast<const unsigned char *>(str.c_str());
	while (*reading)
	{
        char32_t temp;

		if (*reading < 0b10000000U)
        {
            result.push_back(*reading++);
        }
		else if (*reading < 0b11100000U)
		{
			(temp = *reading++) <<= 8;
			result.push_back(temp |= *reading++);
		}
		else if (*reading < 0b11110000U)
		{
			(temp  = *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			result.push_back(temp |= *reading++);
		}
		else if (*reading < 0b11111000U)
		{
			(temp  = *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			result.push_back(temp |= *reading++);
		}
		else
        {
            break;
        }
    }
    return result;
}

inline std::string
utf8_to_str(const std::u32string &str)
{
    std::string result;
    for (auto chr : str)
    {
        if (chr < 0b10000000U)
        {
            result += chr;
        }
        else if (chr < 0b111U << 13)
        {
            result += chr >> 8 & 0xFF;
            result += chr      & 0xFF;
        }
        else if (chr < 0b1111U << 20)
        {
            result += chr >> 16 & 0xFF;
            result += chr >>  8 & 0xFF;
            result += chr       & 0xFF;
        }
        else if (chr < 0b11111U << 27)
        {
            result += chr >> 24 & 0xFF;
            result += chr >> 16 & 0xFF;
            result += chr >>  8 & 0xFF;
            result += chr       & 0xFF;
        }
        else break;
    }
    return result;
}

using Scope  = std::pair<char32_t, char32_t>;
using NFAPtr = std::shared_ptr<struct NFAState>;
using DFAPtr = std::shared_ptr<struct DFAState>;

inline std::set<Scope>
SPACES
{
    { 9, 13 },
    { 32, 32 }
};

inline std::set<Scope>
NOT_SPACES
{
    { kChar32Min, 8 },
    { 14, 31 },
    { 33, kChar32Max }
};

inline std::set<Scope>
DIGITS
{
    { 48, 57 }
};

inline std::set<Scope>
NOT_DIGITS
{
    { kChar32Min, 47 },
    { 58, kChar32Max }
};

inline std::set<Scope>
LWORDS
{
    { 97, 122 }
};

inline std::set<Scope>
NOT_LWORDS
{
    { kChar32Min, 96 },
    { 121, kChar32Max }
};

inline std::set<Scope>
UWORDS
{
    { 65, 90 }
};

inline std::set<Scope>
NOT_UWORDS
{
    { kChar32Min, 64 },
    { 91, kChar32Max }
};

inline std::set<Scope>
WORD_S
{
    { 48, 57 },
    { 65, 90 },
    { 95, 95 },
    { 97, 122 }
};

inline std::set<Scope>
NOT_WORD_S
{
    { kChar32Min, 47 },
    { 58, 64 },
    { 91, 94 },
    { 96, 96 },
    { 121, kChar32Max }
};

inline std::unordered_map<char32_t, std::set<Scope>>
ECMAP
{
    {'s', SPACES}, {'S', NOT_SPACES},
    {'d', DIGITS}, {'D', NOT_DIGITS},
    {'l', LWORDS}, {'L', NOT_LWORDS},
    {'u', UWORDS}, {'U', NOT_UWORDS},
    {'w', WORD_S}, {'W', NOT_WORD_S}
};

struct NFAState
{
    enum class EdgeType
    {
        EPSILON, CCL, EMPTY
    };

    EdgeType edge_type;
    std::set<Scope> scopes;
    NFAPtr next;
    NFAPtr next2;

    NFAState() : edge_type(EdgeType::EMPTY), next(nullptr), next2(nullptr) {}

    bool contains_scope(const Scope &scope)
    {
        for (const auto &s : scopes)
        {
            if (s.first <= scope.first && scope.second <= s.second)
            {
                return true;
            }
        }
        return false;
    }
};

struct DFAState
{
    enum class State
    {
        NORMAL, END
    };

    State state;
    std::map<Scope, DFAPtr> scope_state;

    DFAState() : state(State::NORMAL) {}
    DFAState(State state) : state(state) {}

    bool contains_scope(const Scope &scope)
    {
        for (const auto &scope_s : scope_state)
        {
            if (scope_s.first.first <= scope.first && scope.second <= scope_s.first.second)
            {
                return true;
            }
        }
        return false;
    }

    bool contains_scope(char32_t chr)
    {
        for (const auto &scope_s : scope_state)
        {
            if (scope_s.first.first <= chr && chr <= scope_s.first.second)
            {
                return true;
            }
        }
        return false;
    }

    DFAPtr get_next(const Scope &scope)
    {
        for (const auto &scope_s : scope_state)
        {
            if (scope_s.first.first <= scope.first && scope.second <= scope_s.first.second)
            {
                return scope_s.second;
            }
        }
        return nullptr;
    }

    DFAPtr get_next(char32_t chr)
    {
        for (const auto &scope_s : scope_state)
        {
            if (scope_s.first.first <= chr && chr <= scope_s.first.second)
            {
                return scope_s.second;
            }
        }
        return nullptr;
    }
};

class NFAPair
{
  public:
    NFAPtr start;
    NFAPtr end;

    NFAPair() : start(std::make_shared<NFAState>()), end(std::make_shared<NFAState>()) {}
    NFAPair(std::shared_ptr<NFAState> start, std::shared_ptr<NFAState> end) : start(start), end(end) {}

    DFAPtr to_dfa()
    {
        auto q0 = eps_closure({ start });
        std::vector<std::set<NFAPtr>> Q = {q0};
        std::list<std::set<NFAPtr>> work_list= {q0};
        std::vector<DFAPtr> mp = {std::make_shared<DFAState>(
            (std::find(q0.begin(), q0.end(), end) != q0.end()) 
            ? DFAState::State::END
            : DFAState::State::NORMAL
        )};

        while (!work_list.empty())
        {
            auto q = work_list.back();
            work_list.pop_back();

            for (auto scope : cal_scopes(q))
            {
                auto t = eps_closure(delta(q, scope));
                if (t.empty())
                {
                    continue;
                }

                for (std::size_t i = 0, j = 0; i < Q.size(); ++i)
                {
                    if (Q[i] == q)
                    {
                        while (j < Q.size())
                        {
                            if (Q[j] == t)
                            {
                                mp[i]->scope_state[scope] = mp[j];
                                break;
                            }
                            else
                            {
                                ++j;
                            }
                        }

                        if (j == Q.size())
                        {
                            Q.push_back(t);
                            work_list.push_back(t);
                            mp.push_back(std::make_shared<DFAState>(
                                (std::find(t.begin(), t.end(), end) != t.end())
                                ? DFAState::State::END
                                : DFAState::State::NORMAL
                            ));
                            mp[i]->scope_state[scope] = mp.back();
                        }

                        break;
                    }
                }
            }
        }

        return dfa_minimization(mp);
    }

  private:
    std::vector<Scope> cal_scopes(std::vector<Scope> &scopes)
    {
        std::vector<Scope> result;
        if (scopes.empty()) return result;

        enum IndexType
        {
            Start, End
        };
        std::vector<std::pair<char32_t, IndexType>> indexs;
        
        for (const auto &scope : scopes)
        {
            indexs.push_back({ scope.first, Start });
            indexs.push_back({ scope.second, End });
        }

        std::sort(indexs.begin(), indexs.end(), [](const std::pair<char32_t, IndexType> &a, std::pair<char32_t, IndexType> &b)
        {
            if (a.first < b.first)
            {
                return true;
            }
            else if (a.first == b.first)
            {
                return a.second < b.second;
            }
            else 
            {
                return false;
            }
        });

        char32_t start;
        int left = 0;
        for (auto it = indexs.begin(); it != indexs.end(); ++it)
        {
            if (left == 0)
            {
                start = it->first;
                ++left;
            }
            else if (it->second == Start)
            {
                if (it->first == start)
                {
                    ++left;
                    continue;
                }
                result.push_back({ start, it->first - 1 });
                ++left;
                start = it->first;
            }
            else // it->second == End
            {
                if (it->first < start)
                {
                    --left;
                    continue;
                }
                result.push_back({ start, it->first });
                --left;
                if (left > 0)
                {
                    start = it->first + 1;
                }
            }
        }

        return result;
    }

    std::vector<Scope> cal_scopes(const std::set<NFAPtr> &q)
    {
        std::vector<Scope> temp;
        for (auto &state : q)
        {
            for (auto &scope : state->scopes)
            {
                temp.push_back(scope);
            }
        }
        return cal_scopes(temp);
    }

    std::vector<Scope> cal_scopes(const std::set<DFAPtr> &q)
    {
        std::vector<Scope> temp;
        for (auto &state : q)
        {
            for (auto &scope_s : state->scope_state)
            {
                temp.push_back(scope_s.first);
            }
        }
        return cal_scopes(temp);
    }

    void add2rS(std::set<NFAPtr> &rS, const NFAPtr &s)
    {
        rS.insert(s);
        if (s->edge_type == NFAState::EdgeType::EPSILON)
        {
            if (!rS.count(s->next))
            {
                add2rS(rS, s->next);
            }
            if (s->next2 && !rS.count(s->next2))
            {
                add2rS(rS, s->next2);
            }
        }
    }

    std::set<NFAPtr>
    eps_closure(const std::set<NFAPtr> &S)
    {
        std::set<NFAPtr> rS;
        for (auto &s: S)
        {
            add2rS(rS, s);
        }
        return rS;
    }

    std::set<NFAPtr>
    delta(std::set<NFAPtr> &q, const Scope &scope)
    {
        std::set<NFAPtr> rq;
        for (auto &s : q)
        {
            if (s->edge_type == NFAState::EdgeType::CCL && s->contains_scope(scope))
            {
                rq.insert(s->next);
            }
        }
        return rq;
    }

    DFAPtr
    dfa_minimization(std::vector<DFAPtr> &mp)
    {
        std::set<std::set<int>> T, P;

        auto indexof_inmp = [&](std::shared_ptr<DFAState> state)
        {
            for (std::size_t i = 0; i < mp.size(); ++i)
            {
                if (mp[i] == state)
                {
                    return i;
                }
            }
            return std::numeric_limits<std::size_t>::max();
        };

        {
            std::vector<std::set<int>> _T = {{}, {}};

            for (std::size_t i = 0; i < mp.size(); ++i)
            {
                _T[mp[i]->state == DFAState::State::END].insert(i);
            }

            T.insert(_T[0]); T.insert(_T[1]);
        }

        auto split = [&](const std::set<int> &S)
        {
            std::vector<std::set<int>> res = {S};
            std::set<DFAPtr> dfaptrs;
            for (auto i : S)
            {
                dfaptrs.insert(mp[i]);
            }

            for (auto scope : cal_scopes(dfaptrs))
            {
                std::set<int> s1, s2;

                auto flag_it = P.end();

                for (auto i: S)
                {
                    if (mp[i]->contains_scope(scope))
                    {
                        int k = indexof_inmp(mp[i]->get_next(scope));
                        for (auto it = P.begin(); it != P.end(); ++it)
                        {
                            if (it->count(k))
                            {
                                if (it == flag_it)
                                {
                                    s1.insert(i);
                                }
                                else if (flag_it == P.end())
                                {
                                    flag_it = it;
                                    s1.insert(i);
                                }
                                else
                                {
                                    s2.insert(i);
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        s2.insert(i);
                    }
                }

                if (s1.size() && s2.size())
                {
                    res = {s1, s2};
                    return res;
                }
            }
            return res;
        };

        while (P != T)
        {
            P = T;
            T.clear();
            for (auto &p: P)
            {
                for (auto &_p: split(p))
                {
                    T.insert(_p);
                }
            }
        }

        std::vector<std::shared_ptr<DFAState>> states;
        for (std::size_t i = 0; i < T.size(); ++i)
        {
            states.push_back(std::make_shared<DFAState>());
        }
        std::shared_ptr<DFAState> start = nullptr;

        {
            std::vector<std::set<int>> P(T.begin(), T.end());

            auto indexof_inp = [&](std::shared_ptr<DFAState> state)
            {
                for (std::size_t i = 0, k = indexof_inmp(state); i < P.size(); ++i)
                {
                    if (P[i].count(k))
                    {
                        return i;
                    }
                }
                return std::numeric_limits<std::size_t>::max();
            };

            for (std::size_t i = 0; i < P.size(); ++i)
            {
                for (auto &k: P[i])
                {
                    if (mp[k]->state == DFAState::State::END)
                    {
                        states[i]->state = DFAState::State::END;
                    }
                    if (k == 0)
                    {
                        start = states[i];
                    }
                    for (auto scope_s: mp[k]->scope_state)
                    {
                        states[i]->scope_state[scope_s.first] = states[indexof_inp(scope_s.second)];
                    }
                }
            }
        }
        return start;
    }
};

class Node
{
  public:
    virtual ~Node() {}

    virtual std::shared_ptr<NFAPair>
    compile() = 0;
};

class LeafNode : public Node
{
  private:
    char32_t leaf;

  public:
    LeafNode(char32_t c) : leaf(c) {}
    
    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto ptr = std::make_shared<NFAPair>();

        ptr->start->edge_type = NFAState::EdgeType::CCL;
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;
        ptr->start->next = ptr->end;
        ptr->start->scopes.insert({ leaf, leaf });

        return ptr;
    }
};

class CatNode : public Node
{
  private:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

  public:
    CatNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}

    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto left = this->left->compile();
        auto right = this->right->compile();
        auto ptr = std::make_shared<NFAPair>(left->start, right->end);

        left->end->edge_type = NFAState::EdgeType::EPSILON;
        left->end->next = right->start;

        return ptr;
    }
};

class SelectNode : public Node
{
  private:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

  public:
    SelectNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) : left(left), right(right) {}

    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto left = this->left->compile();
        auto right = this->right->compile();
        auto ptr = std::make_shared<NFAPair>();

        ptr->start->edge_type = NFAState::EdgeType::EPSILON;
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;
        ptr->start->next = left->start;
        ptr->start->next2 = right->start;

        left->end->edge_type = NFAState::EdgeType::EPSILON;
        right->end->edge_type = NFAState::EdgeType::EPSILON;
        left->end->next = ptr->end;
        right->end->next = ptr->end;

        return ptr;
    }
};

class ClosureNode : public Node
{
  private:
    std::shared_ptr<Node> content;

  public:
    ClosureNode(std::shared_ptr<Node> content) : content(content) {}

    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto content = this->content->compile();
        auto ptr = std::make_shared<NFAPair>();

        ptr->start->edge_type = NFAState::EdgeType::EPSILON;
        ptr->start->next = content->start;
        ptr->start->next2 = ptr->end;
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;

        content->end->edge_type = NFAState::EdgeType::EPSILON;
        content->end->next = content->start;
        content->end->next2 = ptr->end;

        return ptr;
    }
};

class QualifierNode : public Node
{
  private:
    std::shared_ptr<Node> content;
    int n;
    int m;

  public:

    QualifierNode(std::shared_ptr<Node> content, int n, int m) : content(content), n(n), m(m) {}

    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto ptr = std::make_shared<NFAPair>();
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;

        // -2 means '{n}', -1 means '{n,}', >=0 means '{n,m}'
        if (m == -2) // for '{n}'
        {
            std::shared_ptr<Node> temp = (n > 0) ? content : nullptr;
            for (int i = 1; i < n; ++i)
            {
                temp = std::make_shared<CatNode>(temp, content);
            }
            if (temp)
            {
                return temp->compile();
            }
            else
            {
                ptr->start->edge_type = NFAState::EdgeType::EPSILON;
                ptr->start->next = ptr->end;
            }
        }
        else if (m == -1) // for '{n,}'
        {
            std::shared_ptr<Node> temp = (n > 0) ? content : nullptr;
            for (int i = 1; i < n; ++i)
            {
                temp = std::make_shared<CatNode>(temp, content);
            }
            return temp
                   ? std::make_shared<CatNode>(temp, std::make_shared<ClosureNode>(content))->compile()
                   : std::make_shared<ClosureNode>(content)->compile();
        }
        else if (n < m && n >= 0) // for '{n,m}'
        {
            auto first = content->compile();
            auto pre = first;
            ptr->start->edge_type = NFAState::EdgeType::EPSILON;
            ptr->start->next = first->start;
            if (n == 0)
            {
                ptr->start->next2 = ptr->end;
            }

            for (int i = 1; i < m; ++i)
            {
                auto now = content->compile();
                pre->end->edge_type = NFAState::EdgeType::EPSILON;
                pre->end->next = now->start;
                if (i > n - 2)
                {
                    pre->end->next2 = ptr->end;
                }
                pre = now;
            }

            pre->end->edge_type = NFAState::EdgeType::EPSILON;
            pre->end->next = ptr->end;
        }
        else
        {
            ptr->start->edge_type = NFAState::EdgeType::EPSILON;
            ptr->start->next = ptr->end;
        }

        return ptr;
    }
};

class DotNode : public Node
{
  public:
    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto ptr = std::make_shared<NFAPair>();

        ptr->start->edge_type = NFAState::EdgeType::CCL;
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;
        ptr->start->next = ptr->end;
        ptr->start->scopes.insert({ kChar32Min, 31ULL });
        ptr->start->scopes.insert({ 33ULL, kChar32Max });

        return ptr;
    }
};

class BracketNode : public Node
{
  private:
    std::set<Scope> scopes;

  public:
    BracketNode(std::set<Scope> scopes) : scopes(scopes) {}

    virtual std::shared_ptr<NFAPair>
    compile()
    {
        auto ptr = std::make_shared<NFAPair>();

        ptr->start->edge_type = NFAState::EdgeType::CCL;
        ptr->end->edge_type = NFAState::EdgeType::EMPTY;
        ptr->start->next = ptr->end;
        ptr->start->scopes.insert(scopes.begin(), scopes.end());

        return ptr;
    }
};

class Parse
{
  private:
    bool begin = false;
    bool end   = false;
    std::unordered_map<std::string, std::shared_ptr<Node>> ref_map;
    
    char32_t
    translate_escape_chr(const char32_t *&reading)
    {
        ++reading;
        if (*reading)
        {
            switch (*reading)
            {
            case '0':
                return '\0';
            case 'a':
                return '\a';
            case 'b':
                return '\b';
            case 't':
                return '\t';
            case 'n':
                return '\n';
            case 'v':
                return '\v';
            case 'f':
                return '\f';
            case 'r':
                return '\r';
            case 'e':
                return '\e';
            case 'c':
                if (*(reading + 1) && (isalpha(*(reading + 1)) || (*(reading + 1) > 63 && *(reading + 1) < 94)))
                {
                    ++reading;
                    return toupper(*reading) - 64;
                }
                else
                {
                    return 'c';
                }
            default:
                return *reading;
            }
        }
        else
        {
            return kChar32Max;
        }
    }

    std::set<Scope>
    translate_echr2scopes(char32_t &left, const char32_t *&reading, bool range)
    {
        auto res = translate_escape_chr(reading);
        std::set<Scope> ret;
        if (ECMAP.count(res))
        {
            ret = ECMAP[res];
        }
        else
        {
            left = res;
        }
        if (!range && ECMAP.count(res))
        {
            left = kChar32Max;
        }
        return ret;
    }

    std::shared_ptr<Node>
    translate_echr2node(const char32_t *&reading)
    {
        std::shared_ptr<Node> node = nullptr;
        char32_t left = kChar32Max;
        auto res = translate_echr2scopes(left, reading, false);
        if (left == kChar32Max)
        {
            node = std::make_shared<BracketNode>(res);
        }
        else
        {
            node = std::make_shared<LeafNode>(left);
        }
        return node;
    }

    std::shared_ptr<Node>
    gen_bracket(const char32_t *&reading)
    {
        char32_t left = kChar32Max;
        bool range = false, exclude = false;
        std::set<Scope> scopes;

        if (*reading == '^')
        {
            ++reading;
            exclude = true;
        }
        if (*reading == ']')
        {
            return std::make_shared<BracketNode>(scopes);
        }

        while (*reading && *reading != ']')
        {
            if (*reading == '-')
            {
                if (!range && left != kChar32Max)
                {
                    range = true;
                }
            }
            else if (range)
            {
                if (*reading == '\\')
                {
                    auto res = translate_echr2scopes(left, reading, true);
                    scopes.insert(res.begin(), res.end());
                }
                else
                {
                    scopes.insert({ left, *reading });
                }
                left = kChar32Max;
                range = false;
            }
            else
            {
                if (left != kChar32Max)
                {
                    scopes.insert({ left, left });
                }
                if (*reading == '\\')
                {
                    auto res = translate_echr2scopes(left, reading, false);
                    scopes.insert(res.begin(), res.end());
                }
                else
                {
                    left = *reading;
                }
            }
            ++reading;
        }

        if (left != kChar32Max)
        {
            scopes.insert({ left, left });
        }

        if (exclude)
        {
            std::vector<Scope> sorted_scopes(scopes.begin(), scopes.end());
            std::sort(sorted_scopes.begin(), sorted_scopes.end(), [](Scope a, Scope b)
            {
                return a.first < b.first;
            });

            decltype(scopes) temp;
            char32_t start = kChar32Min;
            for (auto scope : sorted_scopes)
            {
                if (start < scope.first)
                {
                    temp.insert({ start, scope.first - 1 });
                    start = scope.second + 1;
                }
                else if (scope.second >= start)
                {
                    start = scope.second + 1;
                }
            }
            temp.insert({ start, kChar32Max });

            scopes = temp;
        }

        return std::make_shared<BracketNode>(scopes);
    }

    std::shared_ptr<Node>
    gen_subexpr(const char32_t *&reading)
    {
        std::shared_ptr<Node> node = nullptr;
        if (*reading == '?')
        {
            ++reading;
            if (*reading == ':')
            {
                ++reading;
            }
            if (*reading == '<')
            {
                ++reading;
            }
            std::string name;
            while (isalnum(*reading) || *reading == '_')
            {
                name += *reading++;
            }
            if (*reading == '>')
            {
                ++reading;
            }
            if (*reading == ')')
            {
                if (ref_map.count(name))
                {
                    return ref_map[name];
                }
            }
            else
            {
                node = gen_node(reading);
                ref_map[name] = node;
            }
        }
        else
        {
            node = gen_node(reading);
        }
        return node;
    }

    std::shared_ptr<Node>
    gen_node(const char32_t *&reading)
    {
        std::shared_ptr<Node> node = nullptr, right = nullptr;

        if (*reading == '^')
        {
            ++reading;
            begin = true;
        }

        if (*reading == '(')
        {
            ++reading;
            node = gen_subexpr(reading);
        }
        else if (*reading == '[')
        {
            ++reading;
            node = gen_bracket(reading);
        }
        else if (*reading == '.')
        {
            node = std::make_shared<DotNode>();
        }
        else if (*reading && *reading != '|' && *reading != ')')
        {
            node = *reading == '\\'
                ? translate_echr2node(reading) 
                : std::make_shared<LeafNode>(*reading);
        }

        if (!node)
        {
            return node;
        }
        ++reading;

        while (*reading && *reading != '|' && *reading != ')' && *reading != '$')
        {
            switch (*reading)
            {
            case '(':
                ++reading;
                if (right)
                {
                    node = std::make_shared<CatNode>(node, right);
                }
                right = gen_subexpr(reading);
                break;
            case '[':
                ++reading;
                if (right)
                {
                    node = std::make_shared<CatNode>(node, right);
                }
                right = gen_bracket(reading);
                break;
            case '{':
                ++reading;
                if (isdigit(*reading))
                {
                    int n = *reading - '0', m = -2;
                    ++reading;
                    if (*reading == ',')
                    {
                        ++reading;
                        m = isdigit(*reading)
                            ? *reading++ - '0'
                            : -1;
                    }

                    if (right)
                    {
                        right = std::make_shared<QualifierNode>(right, n, m);
                    }
                    else
                    {
                        node = std::make_shared<QualifierNode>(node, n, m);
                    }
                }
                break;
            case '*':
                if (right)
                {
                    right = std::make_shared<ClosureNode>(right);
                }
                else
                {
                    node = std::make_shared<ClosureNode>(node);
                }
                break;
            case '+':
                if (right)
                {
                    right = std::make_shared<QualifierNode>(right, 1, -1);
                }
                else
                {
                    node = std::make_shared<QualifierNode>(node, 1, -1);
                }
                break;
            case '?':
                if (right)
                {
                    right = std::make_shared<QualifierNode>(right, 0, 1);
                }
                else
                {
                    node = std::make_shared<QualifierNode>(node, 0, 1);
                }
                break;
            case '.':
                if (right)
                {
                    node = std::make_shared<CatNode>(node, right);
                }
                right = std::make_shared<DotNode>();
                break;
            default:
                if (right)
                {
                    node = std::make_shared<CatNode>(node, right);
                }
                right = *reading == '\\'
                    ? translate_echr2node(reading)
                    : std::make_shared<LeafNode>(*reading);
                break;
            }
            ++reading;
        }

        if (*reading == '|')
        {
            ++reading;
            if (right)
            {
                node = std::make_shared<CatNode>(node, right);
            }
            node = std::make_shared<SelectNode>(node, gen_node(reading));
        }
        else if (right)
        {
            node = std::make_shared<CatNode>(node, right);
        }

        if (*reading == '$')
        {
            ++reading;
            end = true;
        }
        return node;
    }

  public:
    Parse() {}

    std::tuple<std::shared_ptr<DFAState>, bool, bool>
    gen_dfa(const char32_t *reading)
    {
        std::shared_ptr<DFAState> dfa;

        auto node = gen_node(reading);
        dfa = node
            ? node->compile()->to_dfa()
            : std::make_shared<DFAState>(DFAState::State::END);

        return std::make_tuple(dfa, begin, end);
    }
};
} // namespace details

class Pattern
{
  private:

    void cal_next()
    {
        if (dfa->scope_state.empty())
        {
            return;
        }

        std::set<details::DFAPtr> caled = {dfa};
        std::vector<details::DFAPtr> states;

        for (auto it: dfa->scope_state)
        {
            if (!caled.count(it.second))
            {
                next[it.second] = dfa;
                caled.insert(it.second);
                states.push_back(it.second);
            }
        }

        while (states.size())
        {
            for (auto state: states) 
            {
                for (auto it: state->scope_state)
                {
                    if (!caled.count(it.second))
                    {
                        auto _s = state;
                        while (_s != dfa)
                        {
                            if (next[_s]->contains_scope(it.first))
                            {
                                next[it.second] = _s;
                                break;
                            }
                            else _s = next[_s];
                        }
                        if (!next.count(it.second)) next[it.second] = dfa;
                    }
                }
            }

            std::vector<details::DFAPtr> _ss;
            for (auto state: states) 
            {
                for (auto it: state->scope_state) 
                {
                    if (!caled.count(it.second))
                    {
                        _ss.push_back(it.second);
                        caled.insert(it.second);
                    }
                }
            }
            states = _ss;
        }
    }

    details::DFAPtr dfa;
    std::unordered_map<details::DFAPtr, details::DFAPtr> next;
    bool begin;
    bool end;

  public:
    Pattern(const std::string &pattern)
    {
        auto str = details::str_to_utf8(pattern);
        std::tie(dfa, begin, end) = details::Parse().gen_dfa(str.c_str());
        cal_next();
    }

    std::string
    match(const std::string &str)
    {
        std::u32string u32str = details::str_to_utf8(str),
                       res, temp;
        auto reading = u32str.c_str();
        auto state = dfa;

        while (*reading)
        {
            if (state->contains_scope(*reading))
            {
                state = state->get_next(*reading);
            }
            else if (end)
            {
                return "";
            }
            else
            {
                break;
            }

            temp += *reading;

            if (state->state == details::DFAState::State::END)
            {
                res += temp;
                temp.clear();
            }
            ++reading;
        }

        return details::utf8_to_str(res);
    }

    std::string
    search(const std::string &str)
    {
        if (begin)
        {
            return match(str);
        }

        std::unordered_map<details::DFAPtr, std::u32string> mapstr 
        {
            { dfa, std::u32string() }
        };
        std::u32string u32str = details::str_to_utf8(str),
                       res, temp;

        auto reading = u32str.c_str();
        auto state = dfa;

        while (*reading)
        {
            if (state->contains_scope(*reading))
            {
                state = state->get_next(*reading);
                mapstr[state] = (temp += *reading);
                if (state->state == details::DFAState::State::END)
                {
                    res = u32str.substr(reading - u32str.c_str() - temp.size() + 1, temp.size());
                }
            }
            else if (!end && res.size())
            {
                return details::utf8_to_str(res);
            }
            else if (next.count(state))
            {
                state = next[state];
                temp = mapstr[state];
                continue;
            }
            else
            {
                mapstr[state] = temp = details::str_to_utf8("");
            }
            ++reading;
        }
        return end
            ? details::utf8_to_str(temp)
            : details::utf8_to_str(res);
    }

    std::string
    replace(const std::string &str, const std::string &target)
    {
        if (begin)
        {
            return target + str.substr(match(str).size());
        }

        std::unordered_map<details::DFAPtr, std::u32string> mapstr
        {
            { dfa, std::u32string() }
        };
        std::u32string u32str = details::str_to_utf8(str),
                       u32tar = details::str_to_utf8(target),
                       ret, res, temp;

        auto reading = u32str.c_str();
        auto state = dfa;

        while (*reading)
        {
            if (state->contains_scope(*reading))
            {
                state = state->get_next(*reading);
                mapstr[state] = (temp += *reading);
                if (state->state == details::DFAState::State::END)
                {
                    res = u32str.substr(reading - u32str.c_str() - temp.size() + 1, temp.size());
                }
            }
            else if (!end && res.size())
            {
                ret += u32tar + temp.substr(res.size());
                state = dfa;
                res = temp = std::u32string();
                continue;
            }
            else if (next.count(state))
            {
                state = next[state];
                if (mapstr[state].size() < temp.size())
                {
                    ret += temp.substr(mapstr[state].size());
                }
                temp = mapstr[state];
                continue;
            }
            else
            {
                mapstr[state] = temp = std::u32string();
                ret += *reading;
            }
            ++reading;
        }

        if (res.size())
        {
            ret += u32tar + temp.substr(res.size());
        }

        return details::utf8_to_str(ret);
    }

    std::vector<std::string>
    matches(const std::string &str)
    {
        std::vector<std::string> ret;

        std::u32string u32str = details::str_to_utf8(str),
                       res, temp;

        auto reading = u32str.c_str();
        auto state = dfa;

        while (*reading)
        {
            if (state->contains_scope(*reading))
            {
                state = state->get_next(*reading);
            }
            else if (end)
            {
                return {};
            }
            else
            {
                state = dfa;
                if (res.empty())
                {
                    ++reading;
                }
                else
                {
                    ret.push_back(details::utf8_to_str(res));
                }
                res = temp = std::u32string();
                continue;
            }

            temp += *reading;

            if (state->state == details::DFAState::State::END)
            {
                res += temp;
                temp = std::u32string();
            }

            ++reading;
        }

        if (state->state == details::DFAState::State::END && !res.empty())
        {
            ret.push_back(details::utf8_to_str(res));
        }

        return ret;
    }
};

inline std::string
match(const std::string &pattern, const std::string &str)
{
    return Pattern(pattern).match(str);
}

inline std::string
search(const std::string &pattern, const std::string &str)
{
    return Pattern(pattern).search(str);
}

inline std::string
replace(const std::string &pattern, const std::string &str, const std::string &target)
{
    return Pattern(pattern).replace(str, target);
}

inline std::vector<std::string>
matches(const std::string &pattern, const std::string &str)
{
    return Pattern(pattern).matches(str);
}
} // namespace yare

#endif // YETANOTHERREGEX_HPP