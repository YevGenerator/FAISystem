#pragma once
#include <vector>
#include <string>
#include <format>
#include "algos/2SNO/base_algo.hpp"
#include "algos/2SNO/inspecting_sink.hpp"
#include "nodes/NodeInfo.hpp"
#include "matrix_trace.hpp"
#include "writers/matrix_writers.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {

    class Simulation {
        std::uint64_t m_base_time;
        bool m_node_state{false};
        Core::Nodes::KgOutput m_output;
        std::vector<Core::Nodes::InputDataSlot> m_slots;
        ProfessorMatrixBuilder m_matrix;

    public:
        explicit Simulation(std::uint64_t base_time = 10000,
                            std::size_t slot_count = 3,
                            double out_a = 0.84,
                            double slot_a = 0.85,
                            double slot_g = 0.39)
            : m_base_time(base_time), m_output{.c = true, .a = out_a}
        {
            m_slots.resize(slot_count);
            for (auto& s : m_slots) {
                s.theta = base_time;
                s.kgt.b = 0;
                s.kgt.c = true;
                s.kgt.a = slot_a;
                s.kgt.g = slot_g;
                s.alpha = 0.0;
            }

            TimeStepSnapshot step0;
            step0.time_header = std::format("0/{}", base_time);
            for (const auto& s : m_slots) {
                step0.slots.push_back({.alpha = s.alpha, .theta = s.theta});
            }
            step0.st2 = "Dwn";
            step0.to_switch = 0;
            m_matrix.addStep(std::move(step0));
        }

        Simulation& step(std::size_t slot_id, double alpha, std::uint64_t offset_sec, std::uint64_t n = 1) {
            const std::uint64_t current_time = m_base_time + offset_sec;
            m_slots[slot_id].alpha = alpha;
            m_slots[slot_id].theta = current_time;

            Core::Nodes::NodeAlgoInfo nodeInfo{
                .currentInput = {.alpha = alpha, .theta = current_time},
                .inputs = m_slots,
                .output = m_output,
                .state = m_node_state
            };

            TimeStepSnapshot snap;
            snap.time_header = std::format("{}/{}", n, current_time);

            for (const auto& s : m_slots) {
                snap.slots.push_back({.alpha = s.alpha, .theta = s.theta});
            }

            auto inspector = Core::Algo::twoSNO::InspectingSink{
                Core::Algo::twoSNO::steps::DirectSumAccumulator{},
                [&, current_slot = std::size_t{0}](const Core::Algo::twoSNO::steps::CFSlotPipeline::result_t& trace,
                                                   Core::Algo::AlgoFloat current_cf_star) mutable
                {
                    const std::size_t idx = current_slot++;
                    snap.slots[idx].beta = static_cast<double>(trace.time.beta);
                    snap.slots[idx].cf = trace.cf.cf;
                    snap.slots[idx].k_beta = trace.cf.k_beta;
                    snap.slots[idx].ro = trace.ro.ro;
                    snap.cf_star = current_cf_star;
                }
            };

            auto step1_time = Core::Algo::twoSNO::steps::ExtractTime::calculate(nodeInfo);
            auto step2_cf = Core::Algo::twoSNO::steps::CF_Star::calculate(step1_time, nodeInfo.inputs, inspector);
            auto step3_res = Core::Algo::twoSNO::steps::Alpha::calculate(step2_cf, nodeInfo);

            snap.cf_imkg = step2_cf.cf_imkg;
            snap.alpha_imkg = step3_res.alpha;
            snap.t_imkg = current_time;
            snap.alpha_up = step3_res.alpha_up;
            snap.alpha_dwn = step3_res.alpha_down;
            snap.to_switch = step3_res.toSwitch ? 1 : 0;

            if (step3_res.toSwitch) {
                m_node_state = step3_res.next_state;
            }
            snap.st2 = m_node_state ? "Up" : "Dwn";

            if (step3_res.toSwitch) {
                snap.alpha_out = m_node_state ? step3_res.alpha_up : step3_res.alpha_down;
                snap.message = "yes";
            } else {
                snap.alpha_out = step3_res.alpha;
                snap.message = "no";
            }
            snap.t_out = current_time;

            m_matrix.addStep(std::move(snap));
            return *this;
        }

        Simulation& save(const std::string& base_filename = "matrix_result") {
            MatrixExporter::exportAll(base_filename, m_matrix);
            return *this;
        }

        [[nodiscard]] const ProfessorMatrixBuilder& matrix() const {
            return m_matrix;
        }
    };

    inline void runSimulation() {
        Simulation sim(10000);
        sim.step(0, 0.90, 10)
           .step(2, 0.65, 50)
           .save("matrix_result");
    }

}