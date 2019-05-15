# yare
Yet another regular expression, single-header-file, supports UTF-8 and ASCII only.
there is a common regular expression, [commonregex](https://github.com/MU001999/commonregex), supports universal code

---

### Syntax

Metacharacter   | Description
--------------- | -----------
.               | Matches any single character (many applications exclude newlines('\n'). Within bracket expressions, the dot character matches a literal dot. For example, `a.c` matches "abc", etc., but [a.c] matches only "a", ".", or "c".
[...]           | A bracket expression. Matches a single character that is contained within the brackets. For example, `[abc]` matches "a", "b", or "c". `[a-z]` specifies a range which matches any lowercase letter from "a" to "z". These forms can be mixed: `[abcx-z]` matches "a", "b", "c", "x", "y", or "z", as does `[a-cx-z]`.
[^...]          | Matches a single character that is not contained within the brackets. For example, `[^abc]` matches any character other than "a", "b", or "c". `[^a-z]` matches any single character that is not a lowercase letter from "a" to "z". Likewise, literal characters and ranges can be mixed.
^               | Matches the starting position within the string.
$               | Matches the ending position of the string or the position just before a string-ending newline.
\*	            | Matches the preceding element zero or more times. For example, `ab*c` matches "ac", "abc", "abbbc", etc. `[xyz]*` matches "", "x", "y", "z", "zx", "zyx", "xyzzy", and so on. `(ab)*` matches "", "ab", "abab", "ababab", and so on.
\+              | Matches the preceding element one or more times. For example, `ab+c` matches "abc", "abbc", "abbbc", and so on, but not "ac".
?               | Matches the preceding element zero or one time. For example, `ab?c` matches only "ac" or "abc".
\|              | The choice (also known as alternation or set union) operator matches either the expression before or the expression after the operator. For example, `abc|def` matches "abc" or "def".
()              | Defines a marked subexpression. The string matched within the parentheses can be recalled later.
{n}             | Matches the preceding element n times.
{n,}            | Matches the preceding element at least n times.
{n, m}          | Matches the preceding element at least m and not more than n times. For example, `a{3,5}` matches only "aaa", "aaaa", and "aaaaa". This is not found in a few older instances of regexes.
(?\<name\>)     | Matches what the name marked subexpression matched.
(?\<name\>...)  | Defines a marked subexpression. The string matched within the parentheses can be recalled later by the name.
\s              | Matches a whitespace character; same as `[ \f\n\r\t\v]`.
\S              | Matches anything BUT a whitespace; same as `[^ \f\n\r\t\v]`.
\l              | Matches a lowercase letter; same as `[a-z]`.
\L              | Matches anything BUT a lowercase letter; same as `[^a-z]`.
\u              | Matched a uppercase letter; same as `[A-Z]`.
\U              | Matched anything BUT a uppercase letter; same as `[^A-Z]`.
\w              | Matches an alphanumeric character, including "\_"; same as `[A-Za-z0-9_]`.
\W              | Matches a non-alphanumeric character, excluding "\_"; same as `[^A-Za-z0-9_]`.
\d              | Matches a digit; same as `[0-9]`.
\D              | Matches a non-digit; same as `[^0-9]`.
\cX             | Matches a control character indicated by X. For example, \cM matches a Control-M or carriage return. The value of x must be one of A-Z or a-z. Otherwise, treat c as a literal 'c' character.
\0, \a, \b, \t, \n, \v, \f, \r, \r | Matches a escape character.

###### Examples
* `(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}` denotes ipv4 address, such as 192.168.1.1, 255.255.255.0 and so on.

### Interface

###### Class

Class Name | Description
---------- | -----------
Pattern    | Pattern object, provides methods for matching, searching and replacing.

###### Functions

Function Name | Description
------------- | -----------
match         | attempts to match a regular expression to an entire character sequence.
search        | attempts to match a regular expression to any part of a character sequence.
replace       | replaces occurrences of a regular expression with formatted replacement text.
matches       | attempts to match a regular expression to some entire character sequences.

###### Examples

```cpp
// e.g. Pattern's methods
#include "yare.hpp"

auto pattern = yare::Pattern("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}")
auto match_result = pattern.match("192.168.1.1");
auto search_result = pattern.search("ipv4 address: 123.123.123.123");
auto replace_result = pattern.replace("ipv4 address: 123.123.123.123", "***.***.***.***");
auto matches_result = pattern.matches("<meta test1> <meta test2>");
```

```cpp
// e.g. normal functions
#include "yare.hpp"

auto match_result = yare::match("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}", "192.168.1.1");
auto search_result = yare::search("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}", "ipv4 address: 123.123.123.123");
auto replace_result = yare::replace("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}", "ipv4 address: 123.123.123.123", "***.***.***.***");
auto matches_result = yare::matches("<meta[^>]*>", "<meta test1> <meta test2>");
```
