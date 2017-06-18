#include "parsing.hh"
#include <cctype>
#include <cmath>
#include <functional>

namespace Parcival
{
    inline auto char_to_number(char c)
    {
        return result(static_cast<unsigned>(c - '0'));
    }

    inline auto sign_to_number(char c)
    {
        switch (c)
        {
            case '-': return result<int>(-1);
            case '+': return result<int>(+1);
            default: return result_t<int>(failure_t{});
        }
    }

    inline unsigned long add_digit(unsigned long y, unsigned d)
    {
        return y * 10U + (unsigned long)d;
    }

    inline result_t<long> make_signed_integer(int s, unsigned long x)
    {
        return result<long>(s * x);
    }

    auto digit
        = item<char>
       >= predicate<char>(isdigit)
       >= char_to_number;

    auto sign
        = (item<char>
           >= sign_to_number)
        | result<int>(1);

    auto number
        = digit
       >= [] (unsigned long x)
           { return reduce(add_digit, digit, x); };

    auto signed_integer
        = (sign, number) >> make_signed_integer;
}

/*
auto p_integer()
{
    struct fp
    {
        int s;
        long i;
    };

    auto sign = [] (fp value)
    {
        return (item<char>
            >= one_of("+-")
            >= [value] (char c)
            {
                fp v = value;
                switch (c)
                {
                    case '-': v.s = -1; break;
                    case '+': v.s = +1; break;
                }
                return result(v);
            }) | result(value);
    };

    auto digits = [digit] (fp value)
    {
        return digit
            >= [value] (unsigned x)
            {
                fp v = value; v.i = x; return result(v);
            }
            >= [digit] (fp value)
            {
                return reduce([] (fp v, unsigned x)
                {
                    v.i *= 10;
                    v.i += x;
                    return v;
                }, digit, value);
            };
    };

    auto eval = [] (fp value)
    {
        return result(value.i * value.s);
    };

    return result(fp{ 1, 0 }) >= sign >= digits >= eval;
}

auto integer = p_integer(); */