#pragma once
#include <concepts>

#include "types.hpp"
#include "types/basic.hpp"

namespace NodeSystem::Core::Algo
{
    template <std::signed_integral TLong = AlgoLongSigned>
    static constexpr auto to_long(const AlgoInt number)
    {
        return static_cast<TLong>(number);
    }

    static constexpr auto return_sign(const bool predicate,
                                      const AlgoFloat value) -> AlgoFloat
    {
        return predicate ? +value : -value;
    }
}
