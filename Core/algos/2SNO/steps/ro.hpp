#pragma once
#include <cmath>

#include "cf.hpp"
#include "evaluate_time.hpp"
#include "algos/types.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct RO
    {
        struct result_t
        {
            AlgoFloat signed_a;
            AlgoFloat ro;
        };

    private:
        static constexpr AlgoFloat calculate_ro(const AlgoFloat a,
                                                const AlgoFloat cf)
        {
            if ((a < 0 and -1 <= cf and cf <= a) || (a > 0 and a <= cf and cf <= 1))
            {
                return 1;
            }
            if (a > 0 and -1 <= cf and cf <= a)
            {
                return -1 + 2 * (cf + 1) / (a + 1);
            }
            if (a < 0 and a <= cf and cf <= 1)
            {
                return -1 + 2 * (1 - cf) / std::abs(a - 1);
            }
            return AlgoFloat{-0.};
        }

    public:
        static constexpr result_t calculate(
            const CF::result_t& stepResult,
            const Nodes::InputDataSlot& nodeState)
        {
            const auto a = return_sign(nodeState.kgt.c, nodeState.kgt.a);
            const auto ro = calculate_ro(a, stepResult.cf);
            return {
                .signed_a = a,
                .ro = ro,
            };
        }
    };
}
