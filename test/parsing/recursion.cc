#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <utility>

#include "parsing.hh"
#include "numbers.hh"
#include "function_traits.hh"

using namespace Parcival;

inline std::istringstream operator"" _s(char const *c, long unsigned int i)
{
    return std::istringstream(c);
}

template <typename P>
struct ref_t
{
    using is_parser_type = std::true_type;
    using output_type = typename P::output_type;

    P const *p;

    constexpr ref_t(P const *p)
        : p(p)
    {}

    template <typename Stream>
    state_t<Stream, output_type> operator()(Stream &&stream)
    {
        return (*p)(std::forward<Stream>(stream));
    }
};

template <typename P>
constexpr auto ref(P const *p)
{
    return ref_t<P>(p);
}

template <typename F>
struct fix_t
{
    using is_parser_type = std::true_type;
    using parser_type = typename function_traits<F>::return_type;
    using output_type = typename parser_type::output_type;
    using stream_type = typename parser_type::stream_type;

    parser_type const p;

    constexpr fix_t(F f)
        : p(f(ref(&p)))
    {}

    state_t<stream_type, output_type> operator()(stream_type &&stream)
    {
        return p(std::move<stream_type>(stream));
    }
};

template <typename Stream, typename T>
struct parser_t
{
    using is_parser_type = std::true_type;
    using output_type = T;
    using stream_type = Stream;

    virtual state_t<Stream, T> operator()(Stream &&stream) const = 0;
    virtual ~parser_t() {}
};

template <typename Stream, typename P>
struct holder_t: public parser_t<Stream, typename P::output_type>
{
    using base_type = parser_t<Stream, typename P::output_type>;
    using output_type = typename base_type::output_type;

    P p;

    holder_t(P p)
        : p(p)
    {}

    virtual state_t<Stream, output_type> operator()(Stream &&stream) const override
    {
        return p(std::forward<Stream>(stream));
    }
};

template <typename Stream, typename P>
auto new_holder(P &&p)
{
    return std::make_unique<holder_t<Stream, typename std::decay<P>::type>>
        (std::forward<P>(p));
}

template <typename T>
class list: public std::optional<std::pair<T, list<T>>>
{
};

template <typename T>
constexpr list<T> empty_list = std::nullopt;

template <typename T>
constexpr list<T> cons(T x, list<T> xs)
{
    return list<T>(x, xs);
}

template <typename Stream, typename P>
auto dmany(holder_t<Stream, list<P>> self, P p)
{
    using element_type = typename P::output_type;

    return (p >= [self] (element_type x)
    {
        return self >= [x] (list<element_type> xs)
        {
            return result(cons(x, xs));
        };
    })
    | result(empty_list<element_type>);
}

TEST(Recursion, Holder)
{
    using stream_type = std::istringstream;
    std::vector<std::unique_ptr<parser_t<stream_type, long>>> ps;
    ps.emplace_back(new_holder<stream_type>(signed_integer));
    auto r = (*ps.back())("-123"_s);
    ASSERT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), -123);
}
