#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename P1, typename P2>
    struct choice_t
    {
        using is_parser_type = std::true_type;
        using output_type = typename P1::output_type;

        P1 p1;
        P2 p2;

        // template <typename P1_, typename P2_>
        constexpr choice_t(P1 p1, P2 p2)
            : p1(p1)
            , p2(p2)
        {}

        template <typename Stream>
        state_t<typename std::decay<Stream>::type, output_type> operator()(Stream &&stream) const
        {
            auto pos = stream.tellg();
            auto first = p1(std::move(stream));
            if (first.is_success()) {
                // parser moves on, discard previous stream state
                return std::move(first);
            }

            // revert stream to previous position and try second parser
            first.stream().clear();
            first.stream().seekg(pos);
            return p2(std::move(first.stream()));
        }
    };

    template <typename P1, typename P2,
              typename = typename std::enable_if
                  <std::decay<P1>::type::is_parser_type::value>::type,
              typename = typename std::enable_if
                  <std::decay<P2>::type::is_parser_type::value>::type>
    constexpr auto operator|(P1 &&p1, P2 &&p2)
    {
        return choice_t<typename std::decay<P1>::type,
                        typename std::decay<P2>::type>
                (std::forward<P1>(p1), std::forward<P2>(p2));
    }
}
