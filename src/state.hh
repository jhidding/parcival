#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <type_traits>
#include <tuple>
#include <functional>

namespace Parcival
{
    class eof_t {};
    constexpr eof_t eof;

    class error_t: public std::runtime_error
    {
        public:
            error_t(std::string &&msg)
                : std::runtime_error(std::forward<std::string>(msg))
            {}
    };

    inline error_t error(std::string &&msg)
    {
        return error_t(std::forward<std::string>(msg));
    }

    struct failure_t {};

    template <typename Stream, typename Output>
    using state_base = std::pair<Stream, std::variant<Output, failure_t>>;

    template <typename Stream, typename Output>
    class state_t: public state_base<Stream, Output>
    {
        using base_type = state_base<Stream, Output>;
        using var_type = std::variant<Output, failure_t>;

    public:
        using output_type = Output;

        template <typename S, typename O>
        state_t(S &&stream, O &&output)
            : base_type(std::move(stream),
                        std::move(output))
        {}

        template <typename S>
        state_t(S &&stream, failure_t const &f)
            : base_type(std::move(stream), f)
        {}

        state_t(state_t &&s)
            : base_type(std::move(s))
        {}

        state_t &operator=(state_t &&x)
        {
            this->first = std::move(x.first);
            this->second = std::move(x.second);
            return *this;
        }

        constexpr bool is_success() noexcept
        {
            return this->second.index() == 0;
        }

        Stream &stream()
        {
            return this->first;
        }

        Output &value()
        {
            return std::get<0>(this->second);
        }
    };

    template <typename Stream, typename Output>
    state_t<typename std::decay<Stream>::type,
            typename std::decay<Output>::type>
    state(Stream &&stream, Output &&value)
    {
        return state_t<typename std::decay<Stream>::type,
                       typename std::decay<Output>::type>(
            std::forward<Stream>(stream),
            std::forward<Output>(value));
    }

    template <typename T, typename Stream>
    state_t<typename std::decay<Stream>::type, T> failure(Stream &&stream)
    {
        return state_t<typename std::decay<Stream>::type, T>(stream, failure_t{});
    }
}
