#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename P, typename F>
    struct sequence_t
    {
        using is_parser_type = std::true_type;
        using output_type = typename decltype(std::declval<F>()
                (std::declval<typename P::output_type>())
                (std::declval<std::istream>()))::output_type;

        P p;
        F f;

        template <typename P_, typename F_>
        constexpr sequence_t(P_ p, F_ &&f)
            : p(p)
            , f(std::move(f))
        {}

        template <typename Stream>
        state_t<typename std::decay<Stream>::type, output_type> operator()(Stream &&stream) const
        {
            auto step = p(std::move(stream));

            if (not step.is_success())
            {
                return failure<output_type>(std::move(step.stream()));
            }

            auto next_parser = f(std::move(step.value()));
            return next_parser(std::move(step.stream()));
        }
    };

    template <typename P, typename F>
    constexpr auto operator>=(P &&p, F &&f)
    {
        return sequence_t<typename std::decay<P>::type,
                          typename std::decay<F>::type>
            (std::forward<P>(p), std::forward<F>(f));
    }

    template <typename P,
              typename F,
              typename = typename std::enable_if<P::is_parser_type::value>::type>
    constexpr auto operator>>(P p, F f)
    {
        using T = typename P::output_type;

        return p >= [f] (T const &t)
        {
            return std::apply(f, t);
        };
    }
}
