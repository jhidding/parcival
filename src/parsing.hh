#pragma once

#include <stdexcept>
#include <variant>
#include <type_traits>
#include <sstream>
#include <functional>
#include <stack>
#include <memory>
#include <vector>

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
    constexpr result_t<typename std::decay<T>::type> result(T &&value)
    {
        return result_t<typename std::decay<T>::type>(std::forward<T>(value));
    }

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
                return failure<output_type>(std::move(step.stream()));

            auto next_parser = f(std::move(step.value()));
            return next_parser(std::move(step.stream()));
        }
    };

    template <typename P, typename F>
    constexpr sequence_t<typename std::decay<P>::type,
                         typename std::decay<F>::type> operator>=(P &&p, F &&f)
    {
        return sequence_t<typename std::decay<P>::type,
                          typename std::decay<F>::type>(std::forward<P>(p), std::forward<F>(f));
    }

    template <typename P,
              typename F,
              typename = typename std::enable_if
                  <std::is_same
                      <typename P::is_parser_type, std::true_type>::value>::type>
    constexpr auto operator>>(P p, F f)
    {
        using T = typename P::output_type;

        return p >= [f] (T const &t)
        {
            return std::apply(f, t);
        };
    }

    template <typename Stream, typename ...Ps>
    state_t<typename std::decay<Stream>::type, std::tuple<typename Ps::output_type...>>
    parse_tuple(
            Stream &&stream,
            std::tuple<Ps...> p,
            std::tuple<typename Ps::output_type...> &&v,
            std::index_sequence<>)
    {
        return state(std::move(stream), std::move(v));
    }

    template <typename Stream, typename ...Ps, size_t I0, size_t ...Is>
    state_t<typename std::decay<Stream>::type, std::tuple<typename Ps::output_type...>>
    parse_tuple(
            Stream &&stream,
            std::tuple<Ps...> p,
            std::tuple<typename Ps::output_type...> &&v,
            std::index_sequence<I0, Is...>)
    {
        auto r = std::get<I0>(p)(stream);
        if (not r.is_success())
            return failure<std::tuple<typename Ps::output_type...>>
                (std::move(r.stream()));

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
                std::forward<Stream>(stream), p, output_type{},
                std::index_sequence_for<Ps...>{});
        }
    };

    template <typename P1, typename P2,
              typename = typename std::enable_if
                  <P1::is_parser_type::value>::type,
              typename = typename std::enable_if
                  <P2::is_parser_type::value>::type>
    tuple_t<P1, P2> operator,(P1 p1, P2 p2)
    {
        return tuple_t<P1, P2>(p1, p2);
    }


    template <typename ...Ps, typename P, size_t ...Is>
    tuple_t<Ps..., P> extend_tuple(tuple_t<Ps...> t, P p, std::index_sequence<Is...>)
    {
        return tuple_t<Ps..., P>(std::get<Is>(t)..., p);
    }

    template <typename ...Ps, typename P2,
              typename = typename std::enable_if
                  <P2::is_parser_type::value>::type>
    tuple_t<Ps..., P2> operator,(tuple_t<Ps...> p1, P2 p2)
    {
        return extend_tuple(p1, p2, std::index_sequence_for<Ps...>{});
    }

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
            first.stream().seekg(pos);
            return p2(std::move(first.stream()));
        }
    };

    template <typename P1, typename P2,
              typename = typename std::enable_if
                  <std::is_same
                      <typename std::decay<P1>::type::is_parser_type, std::true_type>::value>::type,
              typename = typename std::enable_if
                  <std::is_same
                      <typename std::decay<P2>::type::is_parser_type, std::true_type>::value>::type>
    constexpr choice_t<typename std::decay<P1>::type,
                       typename std::decay<P2>::type>
    operator|(P1 &&p1, P2 &&p2)
    {
        return choice_t<typename std::decay<P1>::type,
                        typename std::decay<P2>::type>
                (std::forward<P1>(p1), std::forward<P2>(p2));
    }

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

    template <typename T>
    struct seq_t: public std::vector<T>
    {
        using base_type = std::vector<T>;
    };

    template <>
    struct seq_t<char>: public std::string
    {
        using base_type = std::string;
    };

    template <typename P>
    struct many_t
    {
        using is_parser_type = std::true_type;
        using element_type = typename P::output_type;
        using output_type = seq_t<element_type>;

        P p;

        template <typename P_>
        constexpr many_t(P_ p)
            : p(p)
        {}

        template <typename Stream>
        state_t<Stream, output_type> operator()(Stream &&stream) const
        {
            seq_t<element_type> r;
            auto pos = stream.tellg();
            auto s1 = p(std::move(stream));

            while (true) {
                if (not s1.is_success())
                {
                    s1.stream().seekg(pos);
                    return state(std::move(s1.stream()), std::move(r));
                }

                r.push_back(std::move(s1.value()));
                pos = s1.stream().tellg();
                s1 = p(std::move(s1.stream()));
            }
        }
    };

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

    template <typename P>
    constexpr auto some(P &&p)
    {
        using element_type = typename P::output_type;

        return many(std::forward<P>(p))
            >= predicate<seq_t<element_type>>([] (auto const &x)
        {
            return not x.empty();
        });
    }

    template <typename P>
    constexpr auto many(P &&p)
    {
        return many_t<typename std::decay<P>::type>(std::forward<P>(p));
    }

    template <typename P>
    constexpr auto expect(P p)
    {
        return [p] (auto d)
        {
            return p >= [d] (auto e)
            {
                return result(d);
            };
        };
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
