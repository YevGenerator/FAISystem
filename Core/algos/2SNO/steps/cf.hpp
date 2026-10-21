#pragma once
#include <cmath>

#include "evaluate_time.hpp"
#include "algos/types.hpp"
#include "algos/2SNO/predefined.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct CF
    {
        struct result_t
        {
            AlgoFloat m_x;
            AlgoFloat k_beta;
            AlgoFloat cf;
        };

    private:
        static constexpr auto m_x(const AlgoFloat x,
                                  const AlgoFloat alpha,
                                  const AlgoFloat beta) -> AlgoFloat
        {
            if (beta <= std::numeric_limits<AlgoFloat>::epsilon())
            {
                return 0.0f;
            }
            return
                std::exp(-std::pow(x - alpha, 2) /
                    std::pow(2 * beta, 2));
        }

        static constexpr auto m_x_mean(const AlgoFloat alpha,
                                       const AlgoFloat beta) -> AlgoFloat
        {
            AlgoFloat sum = 0;
            for (const auto x : Predefined::x_sequence)
            {
                sum += m_x(x, alpha, beta);
            }
            return sum / Predefined::Card;
        }

        static constexpr auto v_beta(const AlgoFloat beta) -> AlgoFloat
        {
            return AlgoFloat{1.0};
        }

        static constexpr auto k_beta(const AlgoFloat mx, const AlgoFloat beta) -> AlgoFloat
        {
            return AlgoFloat{1.0} - v_beta(beta) * mx;
        }

    public:
        static constexpr result_t calculate(
            const EvaluateTime::result_t& stepResult,
            const Nodes::InputDataSlot& nodeState)
        {
            const auto mx = m_x_mean(nodeState.alpha, stepResult.beta_normalized);
            const auto k = k_beta(mx, stepResult.beta_normalized);
            const auto cf = nodeState.alpha * k;
            return
            {
                .m_x = mx,
                .k_beta = k,
                .cf = cf
            };
        }
    };
}
