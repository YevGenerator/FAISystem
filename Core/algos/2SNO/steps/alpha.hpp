#pragma once
#include <cmath>
#include "cf_star.hpp"
#include "algos/types.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct Alpha
    {
        struct result_t
        {
            AlgoFloat alpha;
            AlgoFloat alpha_up;
            AlgoFloat alpha_down;
            bool toSwitch;
            bool next_state;
        };

    private:
        static constexpr auto ro_global(const AlgoFloat alpha,
                                        const AlgoFloat epsilon) -> AlgoFloat
        {
            if ((-1 <= alpha and alpha <= epsilon and epsilon <= 0) ||
                (epsilon <= alpha and alpha <= 1 and epsilon >= 0))
            {
                return 1;
            }
            if (-1 <= alpha and alpha <= epsilon and epsilon > 0)
            {
                return -1 + 2 * (alpha + 1) / (epsilon + 1);
            }
            if (epsilon <= alpha and alpha <= 1 and epsilon < 0)
            {
                return -1 + 2 * (1 - alpha) / std::abs(epsilon - 1);
            }
            return AlgoFloat{-0.};
        }

    public:
        static constexpr result_t calculate(
            const CF_Star::result_t& stepResult,
            const Nodes::NodeAlgoInfo& nodeState)
        {
            const AlgoFloat eps_up = nodeState.output.a;
            const AlgoFloat eps_down = -nodeState.output.a;

            const AlgoFloat alpha_up = ro_global(stepResult.cf_imkg, eps_up);
            const AlgoFloat alpha_down = ro_global(stepResult.cf_imkg, eps_down);
            bool toSwitch = false;
            bool nextState = nodeState.state;

            if (!nodeState.state && alpha_up > Predefined::delta)
            {
                nextState = true;
                toSwitch = true;
            }
            else if (nodeState.state && alpha_down > Predefined::delta)
            {
                nextState = false;
                toSwitch = true;
            }

            return {
                .alpha = stepResult.cf_imkg,
                .alpha_up   = alpha_up,
                .alpha_down = alpha_down,
                .toSwitch   = toSwitch,
                .next_state = nextState
            };
        }
    };
}
