#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename T>
    struct item_t
    {
        using is_parser_type = std::true_type;
        using output_type = T;

        template <typename Stream,
                  typename = typename std::enable_if
                      <std::is_same
                          <typename Stream::char_type, T>::value>::type>
        state_t<Stream, T> operator()(Stream &&stream) const
        {
            auto value = stream.get();

            if (value == std::char_traits<typename Stream::char_type>::eof())
                return failure<typename Stream::char_type>(std::move(stream));

            return state<Stream, T>(std::move(stream), std::move(value));
        }

        std::string name() const
        {
            return "any";
        }
    };

    template <typename T>
    constexpr item_t<T> item;
}
