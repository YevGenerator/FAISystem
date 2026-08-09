#pragma once
#include <cmath>
#include <iostream>
#include <random>

#include "../random/gaussian_noise.hpp"
#include "nodes/NodeInfo.hpp"

namespace NodeSystem::Core::Algo {
    struct AlgoTwoSNO {
        static constexpr auto std_dev = 0.01f;
        static constexpr auto q_min = -1.1f;
        static constexpr auto q_max = -q_min;
        static constexpr auto q_step = 0.1f;
        static constexpr auto Card = static_cast<unsigned>((q_max - q_min) / q_step + 1);

        using AlgoFloat = types::Float;
        using AlgoInt = types::UsInt;

        static constexpr AlgoFloat us_in_second = 1'000'000.0f;
        static constexpr AlgoFloat dead_threshold_seconds = 5.0f;

        static constexpr auto x_sequence = [] {
            std::array<AlgoFloat, Card> arr{};
            for (std::size_t i = 0; i < Card; i++) {
                arr[i] = q_min + q_step * static_cast<AlgoFloat>(i);
            }
            return arr;
        }();


        static constexpr auto cf(const AlgoFloat alpha,
                                 const AlgoFloat k_beta) {
            return alpha * k_beta;
        }

        static constexpr auto k_beta(const AlgoFloat v_beta,
                                     const AlgoFloat m_x_mean) -> AlgoFloat {
            return 1. - v_beta * m_x_mean;
        }

        static constexpr AlgoFloat v(const AlgoInt beta) {
            return 1.0;
        }


        static constexpr auto m_x(const AlgoFloat x,
                                  const AlgoFloat alpha,
                                  const AlgoInt beta) -> AlgoFloat {
            if (beta == 0) {
                return 0;
            }
            return std::exp(-std::pow(x - alpha, 2) / std::pow(2 * beta, 2));
        }

        static constexpr auto m_x(const AlgoFloat x,
                                  const AlgoFloat alpha,
                                  const AlgoFloat beta) -> AlgoFloat {
            if (beta <= std::numeric_limits<AlgoFloat>::epsilon()) {
                return 0.0f;
            }
            return std::exp(-std::pow(x - alpha, 2) / std::pow(2 * beta, 2));
        }

        static constexpr auto m_x_mean(const AlgoFloat alpha,
                                       const AlgoFloat beta) -> AlgoFloat {
            AlgoFloat sum = 0;
            for (const auto x: x_sequence) {
                sum += m_x(x, alpha, beta);
            }
            return sum / Card;
        }

        static constexpr auto m_x_mean(const AlgoFloat alpha,
                                       const AlgoInt beta) -> AlgoFloat {
            AlgoFloat sum = 0;
            for (const auto x: x_sequence) {
                sum += m_x(x, alpha, beta);
            }
            return sum / Card;
        }


        static constexpr auto return_sign(const bool c,
                                          const AlgoFloat a) -> AlgoFloat {
            return c ? +a : -a;
        }

        static constexpr AlgoFloat ro(const AlgoFloat a,
                                      const AlgoFloat cf) {
            if ((a < 0 and -1 <= cf and cf <= a) || (a > 0 and a <= cf and cf <= 1)) {
                return 1;
            }
            if (a > 0 and -1 <= cf and cf <= a) {
                return -1 + 2 * (cf + 1) / (a + 1);
            }
            if (a < 0 and a <= cf and cf <= 1) {
                return -1 + 2 * (1 - cf) / std::abs(a - 1);
            }
            return -0.;
        }

        static constexpr auto direct_sum(const AlgoFloat left,
                                         const AlgoFloat right) -> AlgoFloat {
            if ((left > 0 and right > 0) or (left < 0 and right < 0)) {
                return left + right;
            }
            return (left + right) / (1 - std::min(std::abs(left), std::abs(right)));
        }

        static constexpr auto ro_global(const AlgoFloat alpha,
                                        const AlgoFloat epsilon) -> AlgoFloat {
            if ((-1 <= alpha and alpha <= epsilon and epsilon <= 0) ||
                (epsilon <= alpha and alpha <= 1 and epsilon >= 0)) {
                return 1;
            }
            if (-1 <= alpha and alpha <= epsilon and epsilon > 0) {
                return -1 + 2 * (alpha + 1) / (epsilon + 1);
            }
            if (epsilon <= alpha and alpha <= 1 and epsilon < 0) {
                return -1 + 2 * (1 - alpha) / std::abs(epsilon - 1);
            }
            return -0.;
        }

        template<std::signed_integral TLong = long long>
        static constexpr auto to_long(const AlgoInt number) {
            return static_cast<TLong>(number);
        }

        static constexpr auto process(const Nodes::NodeAlgoInfo &nodeState) -> AlgoResult {
            auto t = to_long(nodeState.currentInput.theta);
            double cf_star{};
            bool first = true;
            int i = 0;
            for (const auto &slot: nodeState.inputs) {
                auto theta_i = to_long(slot.theta);
                auto tau_i = to_long(slot.kgt.b);
                auto beta_i = t - theta_i;
                if (tau_i - beta_i >= 0) {
                    beta_i = 0;
                } else {
                    beta_i -= tau_i;
                }

                auto beta_normalized = std::clamp(
                    static_cast<AlgoFloat>(beta_i) / us_in_second / dead_threshold_seconds, 0.0, 1.0);
                auto m = m_x_mean(slot.alpha, beta_normalized);
                auto v_beta = v(beta_i);
                auto k = k_beta(v_beta, m);
                std::cout << "\tk for input " << i++ << ":\t" << k << "\n";
                auto cf_i = cf(slot.alpha, k);
                std::cout << "\tcf for input " << i << ":\t" << cf_i << "\n";
                auto ro_i = ro(return_sign(slot.kgt.c, slot.kgt.a), cf_i);
                std::cout << "\tro for input " << i << ":\t" << ro_i << "\n";
                if (first) {
                    cf_star = slot.kgt.g * ro_i;
                    first = false;
                } else {
                    cf_star = direct_sum(cf_star, slot.kgt.g * ro_i);
                }
            }

            //auto alpha_out = noiseGenerator(cf_star, std_dev);
            std::cout << "\tcf*" << ":\t" << cf_star << "\n";
            auto alpha = ro_global(cf_star, return_sign(nodeState.output.c, nodeState.output.a));
            bool toSwitch = false;
            if (alpha > nodeState.output.a && !nodeState.state) {
                toSwitch = true;
                nodeState.state = !nodeState.state;
            }

            AlgoResult result;
            result.alpha = alpha;
            result.toSwitch = toSwitch;
            return result;
        }
    };
} // namespace NodeSystem::Core::Algo
