#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace NodeSystem::Testing::Calculate::twoSNO {

    struct AlgoTraceRecord {
        std::uint64_t message_index{0};
        std::uint64_t time_ns{0};
        double input_alpha{0.0};

        std::size_t slot_index{0};
        std::uint64_t theta_i{0};
        std::uint64_t tau_i{0};
        double slot_alpha{0.0};
        bool slot_c{false};
        double slot_a{0.0};
        double slot_g{0.0};

        std::int64_t beta_i{0};
        double beta_norm{0.0};
        double m{0.0};
        double v_beta{0.0};
        double k{0.0};
        double cf_i{0.0};
        double ro_i{0.0};
        double contribution{0.0};
        double current_cf_star{0.0};

        double final_alpha{0.0};
        bool state_switched{false};
        bool current_node_state{false};

        static const std::vector<std::string>& headers() {
            static const std::vector<std::string> cols = {
                "msg_idx", "time_ns", "in_alpha",
                "slot_idx", "theta_i", "tau_i", "slot_alpha", "slot_c", "slot_a", "slot_g",
                "beta_i", "beta_norm", "m", "v_beta", "k", "cf_i", "ro_i", "contrib", "cf_star",
                "final_alpha", "switched", "node_state"
            };
            return cols;
        }
    };

} // namespace NodeSystem::Testing