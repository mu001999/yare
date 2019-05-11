#include <cstdio>
#include <cassert>
#include <iostream>

#include "yare.hpp"


using namespace std;


//--DEFINE HELPFUL MACROS--


#define TEST(NAME)									static int NAME = [](){
#define END											return 0; }();
#define PRTL										printf("running at line %d...\n", __LINE__)
#define ASSERT(PATTERN, STR, TARGET) 				PRTL; assert(yare::match(PATTERN, STR) == TARGET)
#define ASSERT_WP(STR, TARGET) 						PRTL; assert(pattern.match(STR) == TARGET)
#define ASSERT_SC(PATTERN, STR, TARGET) 			PRTL; assert(yare::search(PATTERN, STR) == TARGET)
#define ASSERT_RP(PATTERN, STR, STARGET, TARGET)	PRTL; assert(yare::replace(PATTERN, STR, STARGET) == TARGET)
#define ASSERT_MCS(PATTERN, STR, TARGET)			PRTL; assert(yare::matches(PATTERN, STR) == TARGET)


//--TEST NOrMAL MATCH METHOD--


TEST(BLANK)
	ASSERT("", "", "");
	ASSERT("", "abcdefg", "");
END

TEST(SINGLE_CHAR)
	ASSERT("a", "a", "a");
	ASSERT("a", "b", "");
	ASSERT("z", "z", "z");
	ASSERT("z", "y", "");
	ASSERT("z", "y", "");
	ASSERT("A", "A", "A");
	ASSERT("A", "B", "");
	ASSERT("Z", "Z", "Z");
	ASSERT("Z", "Y", "");
	ASSERT("0", "0", "0");
	ASSERT("0", "1", "");
	ASSERT("9", "9", "9");
	ASSERT("9", "8", "");

	ASSERT(".", "0", "0");
	ASSERT(".", "9", "9");
	ASSERT(".", "A", "A");
	ASSERT(".", "", "");

	ASSERT("\\s+", " \f\n\r\t\vabcdefg", " \f\n\r\t\v");
	ASSERT("\\S+", "abcdEFGHijkLmN_\a\nOPQRSTUVWXYZ", "abcdEFGHijkLmN_\a");
	ASSERT("\\w+", "abcdEFGHijkLmN_\nOPQRSTUVWXYZ", "abcdEFGHijkLmN_");
	ASSERT("\\W+", " \f\n\r\t\v_", " \f\n\r\t\v");
END

TEST(CONCATENATE)
	ASSERT("ab", "ab", "ab");
	ASSERT("ab", "ac", "");
	ASSERT(".abc", "aabc", "aabc");
	ASSERT("a..d", "abcd", "abcd");
	ASSERT("...a", "aaab", "");
END

TEST(SELECT)
	ASSERT("a|b", "a", "a");
	ASSERT("a|b", "b", "b");
	ASSERT("ab|c", "ab", "ab");
	ASSERT("ab|c", "c", "c");
END

TEST(QUALIFIER)
	ASSERT("a(b|c)*", "abbbbbc", "abbbbbc");
	ASSERT("a(b|c)*", "a", "a");

	ASSERT("ab|c*", "ccc", "ccc");

	ASSERT("abb*", "ab", "ab");
	ASSERT("abb*", "a", "");

	ASSERT("233+", "233", "233");
	ASSERT("233+", "23", "");
	ASSERT("23+", "23", "23");

	ASSERT("233?", "233", "233");
	ASSERT("233?", "23", "23");
	ASSERT("233?", "2333", "233");
	ASSERT("233?", "2233", "");

	ASSERT("2.?3+", "2333", "2333");
	ASSERT("2.?3+", "23", "23");
	ASSERT("2.?3+", "2233", "2233");
	ASSERT("2.?3+", "22", "");

	ASSERT(".+@.+", "mu00@jusot.com", "mu00@jusot.com");

	ASSERT("23{0,3}", "2332", "233");
	ASSERT("23{0,3}", "", "");
	ASSERT("2{3}", "222", "222");
	ASSERT("2{3}", "22", "");
	ASSERT("2{3}", "2222", "222");
	ASSERT("2{3,}", "22", "");
	ASSERT("2{3,}", "22222", "22222");

	ASSERT("<meta[^>]+>", "<meta name hahah >", "<meta name hahah >");
END

TEST(BRACKET)
	{
		auto pattern = yare::Pattern("[a-c]+[A-C]");
		ASSERT_WP("abcABC", "abcA");
		ASSERT_WP("cccCCC", "cccC");
		ASSERT_WP("bbbBBB", "bbbB");
		ASSERT_WP("AAA", "");
	}

	{
		auto pattern = yare::Pattern("[a-cA-C]+[D-FfG-K]");
		ASSERT_WP("CcBbAaDEFG", "CcBbAaD");
		ASSERT_WP("ALDEF", "");
	}

	{
		auto pattern = yare::Pattern("[^abc]+");
		ASSERT_WP("a", "");
		ASSERT_WP("defghijk\n \taxixixi", "defghijk\n \t");
	}

	ASSERT("1[0-9]{2}", "168", "168");
END

TEST(EXPR_REF)
	{
		auto pattern = yare::Pattern("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}");
		ASSERT_WP("255.255.255.0", "255.255.255.0");
		ASSERT_WP("256.255.255.0", "");
		ASSERT_WP("192.168.1.1", "192.168.1.1");
	}
END

TEST(START_END)
	ASSERT("2333$", "23332", "");
	ASSERT("2333$", "2333", "2333");
END

TEST(COMPLEX)
	{
		auto pattern = yare::Pattern("(abcdefg|123456789)*|cyyzerono1|suchangdashabi|chaoqunlaogenb|(ab*c)");
		ASSERT_WP("abcdefgabcdefg", "abcdefgabcdefg");
		ASSERT_WP("12345668912345", "");
		ASSERT_WP("cyyzerono1", "cyyzerono1");
		ASSERT_WP("cvvzerono1", "");
		ASSERT_WP("abbbbbbbbc", "abbbbbbbbc");
		ASSERT_WP("ac", "ac");
	}

	{
		auto pattern = yare::Pattern("((a|b|c)+(1|2|3)*0?(abc)?)+");
		ASSERT_WP("abc1230abcdefg", "abc1230abc");
		ASSERT_WP("cccbbbaaadefg", "cccbbbaaa");
	}
END


//--TEST SEARCH METHOD--


TEST(SEARCH_M)
	ASSERT_SC("ab*c+", "aaaaaabbbbaaabababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbababcccccc", "abcccccc");
	ASSERT_SC("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}", "ipv4: 192.168.1.1", "192.168.1.1");
END


//--TEST REPLACE METHOD--

TEST(REPLACE_M)
	ASSERT_RP("[A-Z]+", "ABCDEFGhijklmnOPQrstUVWxyz", "_", "_hijklmn_rst_xyz");
	ASSERT_RP("(?:<sec>25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\\.(?:<sec>)){3}", "ipv4: 192.168.1.1", "***.***.***.***", "ipv4: ***.***.***.***");
END


//--TEST MATCHES METHOD--

TEST(MATCHES_M)
	ASSERT_MCS(
		"a[bc]+",
		"ab cccccab ccccccccaccccb b",
		vector<string>({
			"ab",
			"ab",
			"accccb"
		})
	);
END


int main(int argc, char *argv[])
{
	printf("\ntest pass!\n");

	// system("pause"); // test on win.
	return 0;
}