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
    auto p = reduce([] (unsigned x, char c)
    {
        return x * 10 + static_cast<unsigned>(c - '0');
    }, item<char> >= predicate<char>(isdigit), 0u);

    auto r = p("2674abest"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 2674);

    auto s = many(item<char>)(std::move(r.stream()));
    ASSERT_TRUE(s.is_success());
    EXPECT_EQ(s.value(), "abest");
}

template <typename P>
constexpr auto expect(P p)
{
    return [p] (auto d)
    {
        return p >= [d] (auto e)
        {
            return result(d);
        };
    };
}

predicate_t<char> is(char c)
{
    return predicate<char>([c] (char d) { return c == d; });
}

predicate_t<char> one_of(std::string const &s)
{
    return predicate<char>([s] (char d)
        { return s.find(d) != std::string::npos; });
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
    using namespace std::placeholders;

    struct fp
    {
        unsigned x;
        unsigned e;

        fp(unsigned x, unsigned e = 0)
            : x(x), e(e)
        {}

        float value() const
        {
            return x / pow(10, e);
        }
    };

    auto integer = reduce([] (unsigned x, char c)
    {
        return x * 10 + static_cast<unsigned>(c - '0');
    }, item<char> >= predicate<char>(isdigit), 0u);

    auto decimals = [] (unsigned start)
    {
        return reduce([] (fp f, char c)
        {
            f.x *= 10;
            f.e += 1;
            f.x += static_cast<unsigned>(c - '0');
            return f;
        }, item<char> >= predicate<char>(isdigit), fp(start));
    };

    auto simple_float = integer
        >= expect(item<char> >= is('.'))
        >= decimals;

    auto r = simple_float("3.45"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_NEAR(r.value().value(), 3.45, 1e-6);
}

/*
TEST(Parsing, ScientificFloat)
{
    using namespace std::placeholders;

    struct fp
    {
        int s;
        unsigned x;
        unsigned e;

        fp(unsigned x, unsigned e = 0)
            : s(1), x(x), e(e)
        {}

        float value() const
        {
            return s * x / pow(10, e);
        }
    };

    auto sign = [] (fp value)
    {
        (item<char> >= is('-') | item<char> >= is('+'))
            >= [value] (char c)
            {
                switch (c)
                {
                    case '-': value.s = -1; break;
                    case '+': value.s = +1; break;
                }
                return result(value);
            }) | result(value);
    };

    auto integer = [] (fp start)
    {
        reduce([] (unsigned x, char c)
        {
            return x * 10 + static_cast<unsigned>(c - '0');
        }, item<char> >= predicate<char>(isdigit), fp);
    };

    auto decimals = [] (unsigned start)
    {
        return reduce([] (fp f, char c)
        {
            f.x *= 10;
            f.e += 1;
            f.x += static_cast<unsigned>(c - '0');
            return f;
        }, item<char> >= predicate<char>(isdigit), fp(start));
    };

    auto simple_float = integer
        >= expect(item<char> >= is('.'))
        >= decimals;

    auto r = simple_float("3.45"_s);

    ASSERT_TRUE(r.is_success());
    EXPECT_NEAR(r.value().value(), 3.45, 1e-6);
}*/
