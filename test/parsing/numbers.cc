#include <gtest/gtest.h>
#include "parsing.hh"
#include "numbers.hh"
#include <cctype>
#include <cmath>
#include <functional>

using namespace Parcival;

inline std::istringstream operator"" _s(char const *c, long unsigned int i)
{
    return std::istringstream(c);
}

TEST(Parsing, Integer)
{
    auto r = number("2674abest"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 2674);

    auto s = many(item<char>)(std::move(r.stream()));
    ASSERT_TRUE(s.is_success());
    EXPECT_EQ(s.value(), "abest");
}

TEST(Parsing, SignedInteger)
{
    auto r1 = signed_integer("-123"_s);
    ASSERT_TRUE(r1.is_success());
    EXPECT_EQ(r1.value(), -123);

    auto r2 = signed_integer("+9375490375"_s);
    ASSERT_TRUE(r2.is_success());
    EXPECT_EQ(r2.value(), 9375490375);

    auto r3 = signed_integer("+-3"_s);
    EXPECT_FALSE(r3.is_success());
}


TEST(Parsing, SimpleFloat)
{
    auto r = simple_float("3.45"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_NEAR(r.value(), 3.45, 1e-6);
}

TEST(Parsing, ScientificFloat)
{
    auto r1 = scientific_float("3.45"_s);
    ASSERT_TRUE(r1.is_success());
    EXPECT_NEAR(r1.value(), 3.45, 1e-6);

    auto r2 = scientific_float("0.42e2"_s);
    ASSERT_TRUE(r2.is_success());
    EXPECT_NEAR(r2.value(), 42., 1e-6);

    auto r3 = scientific_float("3.95879e-20"_s);
    ASSERT_TRUE(r3.is_success());
    EXPECT_NEAR(r3.value(), 3.95879e-20, 1e-26);
}

TEST(Parsing, Arrays)
{
    auto comma = item<char> >= is(',');
    auto q = (many(number >= expect(comma)), number)
          >> [] (seq_t<unsigned long> s, unsigned long x)
          {
              s.push_back(x);
              return result(s);
          };
    auto s = q("123,456,789,912"_s);

    ASSERT_TRUE(s.is_success());
    std::vector<int> t{ 123, 456, 789, 912 };
    for (unsigned i = 0; i < 3; ++i)
        EXPECT_EQ(s.value()[i], t[i]);

    auto p = (many(tokenize(number) >= expect(tokenize(comma))),
              tokenize(number))
          >> [] (seq_t<unsigned long> s, unsigned long x)
          {
              s.push_back(x);
              return result(s);
          };
    auto r = p("123, 456 , 789,912"_s);

    ASSERT_TRUE(r.is_success());
    for (unsigned i = 0; i < 3; ++i)
        EXPECT_EQ(r.value()[i], t[i]);
}
