#include <gtest/gtest.h>
#include <functional>
#include <cctype>
#include <sstream>
#include "parsing.hh"
// #include "numbers.hh"

using namespace Parcival;

inline std::istringstream operator"" _s(char const *c, long unsigned int i)
{
    return std::istringstream(c);
}

TEST(Parsing, Basics)
{
    auto r = item<char>("abc"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 'a');
}


TEST(Parsing, Sequence)
{
    auto p = item<char> >= [] (char x)
    {
        return item<char> >= [x] (char y)
        {
            return item<char> >= [x] (char z)
            {
                return result(std::make_pair(x, z));
            };
        };
    };

    auto r = p("abc"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), std::pair('a', 'c'));
}


TEST(Parsing, Choice)
{
    auto p = fail<char> | item<char>;
    auto r = p("abc"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 'a');
}


TEST(Parsing, Predicate)
{
    auto p = item<char> >= predicate<char>(
        [] (char const &x) { return x == 'a'; });
    auto q = item<char> >= predicate<char>(
        [] (char const &x) { return x == '1'; });

    auto r = p("abc"_s);
    auto t = q("abc"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 'a');
    EXPECT_FALSE(t.is_success());
}

TEST(Parsing, Many)
{
    auto p = many(item<char>);
    auto r = p("abc"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), "abc");
}

TEST(Parsing, Some)
{
    auto p = some(item<char> >= predicate<char>(isdigit));
    auto r = p("abc"_s);
    auto s = p("123"_s);

    EXPECT_FALSE(r.is_success());
    ASSERT_TRUE(s.is_success());
    EXPECT_EQ(s.value(), "123");
}

TEST(Parsing, Tuples)
{
    auto p = (item<char>, item<char>, item<char>);
    auto r = p("abc"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), std::make_tuple('a', 'b', 'c'));

    auto q = p >> [] (char a, char b, char c)
    {
        return result(std::string() + a + b + c);
    };
    auto s = q("123"_s);

    ASSERT_TRUE(s.is_success());
    EXPECT_EQ(s.value(), "123");

    auto t = p("42"_s);
    EXPECT_FALSE(t.is_success());
}


TEST(Parsing, Tokenize)
{
    auto p = (tokenize(item<char>), tokenize(item<char>), tokenize(item<char>));
    auto r = p("a   b c"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), std::make_tuple('a', 'b', 'c'));

    auto q = many(tokenize(item<char> >= predicate<char>(islower)));
    auto s = q("a b  c   d  efGH"_s);
    ASSERT_TRUE(s.is_success());
    EXPECT_EQ(s.value(), "abcdef");
}

