#pragma once

#include "state.hh"

namespace Parcival
{
    template <typename Output>
    struct result_t
    {
        using is_parser_type = std::true_type;
        using output_type = Output;

        std::variant<Output, failure_t> value;

        constexpr result_t(Output const &value)
            : value(value)
        {}

        constexpr result_t(Output &&value)
            : value(std::move(value))
        {}

        constexpr result_t(failure_t const &f)
            : value(f)
        {}

        template <typename Stream>
        state_t<Stream, Output> operator()(Stream &&stream) const
        {
            if (value.index() == 0)
                return state(std::forward<Stream>(stream), std::get<0>(value));
            else
                return failure<Output>(stream);
        }

        std::string name() const
        {
            return ".";
        }
    };

    template <typename T>
    struct preturn_t
    {
        using is_parser_type = std::true_type;
        using output_type = T;

        T value;

        constexpr preturn_t(T const &value)
            : value(value)
        {}

        template <typename Stream>
        state_t<Stream, output_type> operator()(Stream &&stream) const
        {
            return state(std::forward<Stream>(stream), value);
        }
    };

    template <typename T>
    constexpr auto preturn(T &&value)
    {
        return preturn_t<typename std::decay<T>::type>(std::forward<T>(value));
    }

    template <typename T>
    constexpr result_t<typename std::decay<T>::type> result(T &&value)
    {
        return result_t<typename std::decay<T>::type>(std::forward<T>(value));
    }
}
