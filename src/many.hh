#pragma once

#include "state.hh"
#include "sequence.hh"

#include <vector>

namespace Parcival
{
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
            size_t pos = stream.tellg();
            auto s1 = p(std::move(stream));

            while (true) {
                if (not s1.is_success())
                {
                    s1.stream().clear();
                    s1.stream().seekg(pos);
                    return state(std::move(s1.stream()), std::move(r));
                }

                r.push_back(std::move(s1.value()));
                pos = s1.stream().tellg();
                s1 = p(std::move(s1.stream()));
            }
        }
    };

    template <typename P>
    constexpr auto some(P &&p)
    {
        using element_type = typename std::decay<P>::type::output_type;

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
}
