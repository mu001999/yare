#ifndef YETANOTHERREGEX_HPP
#define YETANOTHERREGEX_HPP

#include <string>
#include <vector>

namespace yare
{
namespace details
{
    inline std::u32string
    str_to_utf8(const std::string &str);
    
    inline std::string
    utf8_to_str(const std::u32string &str);
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