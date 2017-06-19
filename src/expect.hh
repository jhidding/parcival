#pragma once

#include "result.hh"

namespace Parcival
{
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
}
