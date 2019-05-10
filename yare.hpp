#ifndef YETANOTHERREGEX_HPP
#define YETANOTHERREGEX_HPP

#include <string>
#include <vector>
#include <unordered_set>

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
        std::u32string::value_type temp;

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

struct NFAState
{
    enum class EdgeType {
        EPSLION, CCL, EMPTY
    } edgt_type;

    
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