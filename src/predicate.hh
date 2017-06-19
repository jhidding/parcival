#pragma once

#include "state.hh"
#include "result.hh"

namespace Parcival
{
    template <typename T, typename F>
    struct predicate_t
    {
        F f;
        // std::function<bool(T const &)> f;

        //template <typename F>
        constexpr predicate_t(F f): f(f) {}

        template <typename T_>
        result_t<T> operator()(T_ &&x) const
        {
            if (f(x))
                return result_t<T>(std::forward<T_>(x));
            else
                return result_t<T>(failure_t{});
        }
    };

    template <typename T, typename F>
    constexpr predicate_t<T, F> predicate(F f)
    {
        return predicate_t<T, F>(f);
    }

    constexpr auto is(char c)
    {
        return predicate<char>([c] (char d) { return c == d; });
    }

    constexpr auto one_of(char const *s)
    {
        return predicate<char>([s] (char d)
        {
            return std::string(s).find(d) != std::string::npos;
        });
    }
}
