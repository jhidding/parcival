#pragma once

#include "state.hh"

#include <optional>

namespace Parcival
{
    template <typename P>
    struct optional_t
    {
        using output_type = std::optional<typename P::output_type>;
        P p;

        constexpr optional_t(P p)
            : p(p)
        {}

        template <typename Stream>
        state_t<Stream, output_type> operator()(Stream &&stream) const
        {
            auto r = p(stream);
            return state(
                std::move(p.stream()),
                output_type(p.is_success()
                    ? std::move(p.value())
                    : std::nullopt));
        }
    };

    template <typename P>
    constexpr auto optional(P &&p)
    {
        return optional_t<typename std::decay<P>::type>(std::forward<P>(p));
    }
}
