#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename Stream, typename ...Ps>
    state_t<Stream, std::tuple<typename Ps::output_type...>>
    parse_tuple(
            Stream &&stream,
            std::tuple<Ps...> p,
            std::tuple<typename Ps::output_type...> &&v,
            std::index_sequence<>)
    {
        return state(std::move(stream), std::move(v));
    }

    template <typename Stream, typename ...Ps, size_t I0, size_t ...Is>
    state_t<Stream, std::tuple<typename Ps::output_type...>>
    parse_tuple(
            Stream &&stream,
            std::tuple<Ps...> p,
            std::tuple<typename Ps::output_type...> &&v,
            std::index_sequence<I0, Is...>)
    {
        auto r = std::get<I0>(p)(std::move(stream));
        if (not r.is_success())
        {
            return failure<std::tuple<typename Ps::output_type...>>
                (std::move(r.stream()));
        }
        std::get<I0>(v) = std::move(r.value());
        return parse_tuple(std::move(r.stream()), p, std::move(v), std::index_sequence<Is...>{});
    }

    template <typename ...Ps>
    struct tuple_t
    {
        using is_parser_type = std::true_type;
        using output_type = std::tuple<typename Ps::output_type ...>;

        std::tuple<Ps...> p;

        constexpr tuple_t(Ps ... ps)
            : p(ps...)
        {}

        template <typename Stream>
        state_t<Stream, output_type> operator()(Stream &&stream) const
        {
            return parse_tuple(
                std::move(stream), p, output_type{},
                std::index_sequence_for<Ps...>{});
        }
    };

    template <typename P1, typename P2,
              typename = typename std::enable_if
                  <P1::is_parser_type::value>::type,
              typename = typename std::enable_if
                  <P2::is_parser_type::value>::type>
    constexpr tuple_t<P1, P2> operator,(P1 p1, P2 p2)
    {
        return tuple_t<P1, P2>(p1, p2);
    }


    template <typename ...Ps, typename P, size_t ...Is>
    constexpr tuple_t<Ps..., P> extend_tuple(tuple_t<Ps...> t, P p, std::index_sequence<Is...>)
    {
        return tuple_t<Ps..., P>(std::get<Is>(t.p)..., p);
    }

    template <typename ...Ps, typename P2,
              typename = typename std::enable_if
                  <P2::is_parser_type::value>::type>
    constexpr tuple_t<Ps..., P2> operator,(tuple_t<Ps...> p1, P2 p2)
    {
        return extend_tuple(p1, p2, std::index_sequence_for<Ps...>{});
    }
}
