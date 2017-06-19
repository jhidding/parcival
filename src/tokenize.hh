#pragma once

#include "item.hh"
#include "sequence.hh"
#include "predicate.hh"
#include "expect.hh"
#include "many.hh"

#include <cctype>

namespace Parcival
{
    template <typename P>
    constexpr auto tokenize(P &&p)
    {
        auto space = item<char> >= predicate<char>(isspace);
        return std::forward<P>(p) >= expect(many(space));
    }
}
