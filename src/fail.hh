#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename T>
    struct fail_t
    {
        using is_parser_type = std::true_type;
        using output_type = T;

        template <typename Stream>
        constexpr state_t<Stream, T> operator()(Stream &&stream) const
        {
            return failure<T>(std::forward<Stream>(stream));
        }
    };

    template <typename T>
    constexpr fail_t<T> fail;
}
