#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename T, typename P>
    struct reduce_t
    {
        using is_parser_type = std::true_type;
        using element_type = typename P::output_type;
        using output_type = T;

        P p;
        std::function<T (T, element_type)> f;
        output_type start;

        template <typename P_, typename F>
        constexpr reduce_t(P_ p, F f, output_type const &start)
            : p(p)
            , f(f)
            , start(start)
        {}

        template <typename Stream>
        state_t<Stream, output_type> operator()(Stream &&stream) const
        {
            output_type r = start;
            auto pos = stream.tellg();
            auto s1 = p(std::move(stream));

            while (true) {
                if (not s1.is_success())
                {
                    s1.stream().seekg(pos);
                    return state(std::move(s1.stream()), std::move(r));
                }

                r = f(r, std::move(s1.value()));
                pos = s1.stream().tellg();
                s1 = p(std::move(s1.stream()));
            }
        }
    };

    template <typename T, typename P, typename F>
    constexpr auto reduce(F &&f, P &&p, T const &start)
    {
        return reduce_t<T, typename std::decay<P>::type>(
                std::forward<P>(p), std::forward<F>(f), start);
    }
}
