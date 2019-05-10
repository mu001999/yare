#ifndef YETANOTHERREGEX_HPP
#define YETANOTHERREGEX_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <unordered_set>
#include <unordered_map>

namespace yare
{
namespace details
{
inline std::u32string
str_to_utf8(const std::string &str)
{
    std::u32string result;
    auto reading = reinterpret_cast<const unsigned char*>(str.c_str());
	while (*reading)
	{
        char32_t temp;

		if (*reading < 0b10000000U) result.push_back(*reading++);
		else if (*reading < 0b11100000U)
		{
			(temp = *reading++) <<= 8;
			result += (temp |= *reading++);
		}
		else if (*reading < 0b11110000U)
		{
			(temp = *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			result += (temp |= *reading++);
		}
		else if (*reading < 0b11111000U)
		{
			(temp = *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			(temp |= *reading++) <<= 8;
			result += (temp |= *reading++);
		}
		else break;
    }
    return result;
}

inline std::string
utf8_to_str(const std::u32string &str)
{
    std::string result;
    for (auto chr : str)
    {
        if (chr < 0b10000000U) result += chr;
        else if (chr < 0b111U << 13) (result += chr >> 8 & 0xFF) += chr & 0xFF;
        else if (chr < 0b1111U << 20) ((result += chr >> 16 & 0xFF) += chr >> 8 & 0xFF) += chr & 0xFF;
        else if (chr < 0b11111U << 27) (((result += chr >> 24 & 0xFF) += chr >> 16 & 0xFF) += chr >> 8 & 0xFF) += chr & 0xFF;
        else break;
    }
    return result;
}

using Scope = std::pair<char32_t, char32_t>;

struct NFAState
{
    enum class EdgeType
    {
        EPSLION, CCL, EMPTY
    };

    EdgeType edge_type;
    std::unordered_set<Scope> scopes;
    std::shared_ptr<NFAState> next;
    std::shared_ptr<NFAState> next2;

    NFAState() : next(nullptr), next2(nullptr) {}
};

struct DFAState
{
    enum class State
    {
        NORMAL, END
    };

    State state;
    std::unordered_map<Scope, std::shared_ptr<DFAState>> scope_state;

    DFAState() : state(State::NORMAL) {}
    DFAState(State state) : state(state) {}
};

class NFAPair
{
  public:
    std::shared_ptr<NFAState> start;
    std::shared_ptr<NFAState> end;

    NFAPair() : start(std::make_shared<NFAState>()), end(std::make_shared<NFAState>()) {}
    NFAPair(std::shared_ptr<NFAState> start, std::shared_ptr<NFAState> end) : start(start), end(end) {}

    std::shared_ptr<DFAState> to_dfa();

  private:
    void add2rS(std::unordered_set<std::shared_ptr<NFAState>> &rS, const std::shared_ptr<NFAState> &s)
    {
        rS.insert(s);
        if (s->edge_type == NFAState::EdgeType::EPSLION)
        {
            if (!rS.count(s->next)) add2rS(rS, s->next);
            if (!rS.count(s->next2)) add2rS(rS, s->next2);
        }
    }

    std::unordered_set<std::shared_ptr<NFAState>>
    eps_closure(const std::unordered_set<std::shared_ptr<NFAState>> &S)
    {
        std::unordered_set<std::shared_ptr<NFAState>> rS;
        for (auto &s: S) add2rS(rS, s);
        return rS;
    }

    std::unordered_set<std::shared_ptr<NFAState>>
    delta(std::unordered_set<std::shared_ptr<NFAState>> &q, char32_t c)
    {

    }

    std::shared_ptr<DFAState>
    dfa_minimization(std::vector<std::shared_ptr<DFAState>> &mp)
    {

    }
};
} // namespace details

class Pattern
{
  public:
    Pattern(const std::string &pattern);

    std::string
    match(const std::string &str);

    std::string
    search(const std::string &str);

    std::string
    replace(const std::string &str, const std::string &target);

    std::vector<std::string>
    matches(const std::string &str);
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