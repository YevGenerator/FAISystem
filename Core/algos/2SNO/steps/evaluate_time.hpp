#pragma once
#include <algorithm>
#include <concepts>

#include "extract_time.hpp"
#include "algos/types.hpp"
#include "algos/2SNO/predefined.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct EvaluateTime
    {
        struct result_t
        {
            AlgoLongSigned theta;
            AlgoLongSigned tau;
            AlgoLongSigned beta;
            AlgoFloat beta_normalized;
        };

    private:
        static constexpr AlgoLongSigned compare_beta(const AlgoLongSigned beta, const AlgoLongSigned tau)
        {
            if (tau - beta >= 0)
            {
                return 0;
            }

            return beta - tau;
        }

        static constexpr auto normalize_beta(const AlgoLongSigned beta)
        {
            return std::clamp(
                static_cast<AlgoFloat>(beta) / Predefined::us_in_second / Predefined::dead_threshold_seconds,
                AlgoFloat{0.0}, AlgoFloat{1.0});
        }

    public:
        static constexpr result_t calculate(
            const ExtractTime::result_t& input_theta, const Nodes::InputDataSlot& slot)
        {
            const auto theta = to_long(slot.theta);
            const auto tau = to_long(slot.kgt.b);
            auto beta = input_theta - theta;
            beta = compare_beta(beta, tau);

            const auto beta_normalized = normalize_beta(beta);
            return {
                .theta = theta,
                .tau = tau,
                .beta = beta,
                .beta_normalized = beta_normalized
            };
        }
    };
}
