#pragma once
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

    constexpr preturn_t<long> make_signed_integer(int s, unsigned long x)
    {
        return preturn((long)s * (long)x);
    }

    constexpr auto digit
        = item<char>
       >= predicate<char>(isdigit)
       >= char_to_number;

    constexpr auto sign
        = (item<char>
           >= sign_to_number)
        | preturn(1);

    constexpr auto number
        =  digit
        >= [] (unsigned long x)
            { return reduce(add_digit, digit, x); };

    constexpr auto signed_integer
        =  (sign, number)
        >> make_signed_integer;


    struct float_data
    {
        long mantissa;
        int exponent;

        double value() const {
            return mantissa * pow(10, exponent);
        }
    };

    constexpr auto simple_floating_point
        =  signed_integer
        >= expect(item<char> >= is('.'))
        >= [] (long m) {
            return reduce([] (float_data f, unsigned x) {
                return float_data{
                    f.mantissa * 10 + x,
                    f.exponent - 1};
            }, digit, float_data{m, 0});
        };

    constexpr auto simple_float
        =  simple_floating_point
        >= [] (float_data f) {
            return result<double>(f.value());
        };

    constexpr auto scientific_float
        =  (simple_floating_point,
            (item<char>
                >= one_of("eE")
                >= [] (char ignored) {
                    return signed_integer;
                }) | preturn(0L))
        >> [] (float_data f, int e) {
            f.exponent += e;
            return result<double>(f.value());
        };
}
