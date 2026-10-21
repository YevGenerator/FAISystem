#pragma once
#include <string>
#include <vector>
#include <optional>
#include <format>

namespace NodeSystem::Testing::Calculate::twoSNO {

    struct SlotStateSnapshot {
        double alpha{0.0};
        std::uint64_t theta{0};
        std::optional<double> beta;
        std::optional<double> cf;
        std::optional<double> k_beta;
        std::optional<double> ro;
    };

    struct TimeStepSnapshot {
        std::string time_header;
        std::vector<SlotStateSnapshot> slots;
        std::optional<double> cf_imkg;
        std::optional<double> cf_star;
        std::optional<double> alpha_imkg;
        std::optional<std::uint64_t> t_imkg;
        std::optional<double> alpha_up;
        std::optional<double> alpha_dwn;
        std::string st2{"Dwn"};
        int to_switch{0};
        std::optional<double> alpha_out;
        std::optional<std::uint64_t> t_out;
        std::string message{"no"};
    };

    struct MatrixRow {
        std::string algr;
        std::string phase;
        std::string variable;
        std::vector<std::string> cells;
    };

    class ProfessorMatrixBuilder {
        std::vector<std::string> m_headers;
        std::vector<TimeStepSnapshot> m_steps;

        static std::string fmt(const std::optional<double>& val) {
            return val.has_value() ? std::format("{:.2f}", *val) : "";
        }

        static std::string fmt(const std::optional<std::uint64_t>& val) {
            return val.has_value() ? std::to_string(*val) : "";
        }

    public:
        void addStep(TimeStepSnapshot step) {
            m_headers.push_back(step.time_header);
            m_steps.push_back(std::move(step));
        }

        [[nodiscard]] const std::vector<std::string>& headers() const {
            return m_headers;
        }

        [[nodiscard]] std::vector<MatrixRow> buildRows() const {
            std::vector<MatrixRow> rows;
            const std::size_t num_cols = m_steps.size();
            const std::size_t num_slots = m_steps.empty() ? 0 : m_steps[0].slots.size();

            auto add_row = [&](std::string algr, std::string phase, std::string var, auto getter) {
                MatrixRow row{std::move(algr), std::move(phase), std::move(var), {}};
                row.cells.reserve(num_cols);
                for (const auto& step : m_steps) {
                    row.cells.push_back(getter(step));
                }
                rows.push_back(std::move(row));
            };

            for (std::size_t s = 0; s < num_slots; ++s) {
                const std::string phase_name = std::format("In_{}", s + 1);
                add_row("IMKG", phase_name, "alpha", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? fmt(static_cast<std::optional<double>>(st.slots[s].alpha)) : "";
                });
                add_row("IMKG", phase_name, "theta", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? std::to_string(st.slots[s].theta) : "";
                });
                add_row("IMKG", phase_name, "beta", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? fmt(st.slots[s].beta) : "";
                });
                add_row("IMKG", phase_name, "cf", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? fmt(st.slots[s].cf) : "";
                });
                add_row("IMKG", phase_name, "k_beta", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? fmt(st.slots[s].k_beta) : "";
                });
                add_row("IMKG", phase_name, "rho", [s](const TimeStepSnapshot& st) {
                    return s < st.slots.size() ? fmt(st.slots[s].ro) : "";
                });
            }

            add_row("IMKG", "Step 3", "cf_IMKG", [](const TimeStepSnapshot& st) { return fmt(st.cf_imkg); });
            add_row("IMKG", "Step 3", "cf*",     [](const TimeStepSnapshot& st) { return fmt(st.cf_star); });
            add_row("IMKG", "Step 3", "alpha",   [](const TimeStepSnapshot& st) { return fmt(st.alpha_imkg); });
            add_row("IMKG", "Step 3", "t",       [](const TimeStepSnapshot& st) { return fmt(st.t_imkg); });

            add_row("2SNS", "Step 1", "alpha_Up",  [](const TimeStepSnapshot& st) { return fmt(st.alpha_up); });
            add_row("2SNS", "Step 1", "alpha_Dwn", [](const TimeStepSnapshot& st) { return fmt(st.alpha_dwn); });
            add_row("2SNS", "Step 2", "St_2",      [](const TimeStepSnapshot& st) { return st.st2; });
            add_row("2SNS", "Step 2", "Switch",    [](const TimeStepSnapshot& st) { return std::to_string(st.to_switch); });

            add_row("2SNS O", "Step 4", "alpha_Out", [](const TimeStepSnapshot& st) {
                return (st.time_header == "0/10000" || !st.alpha_out.has_value()) ? "" : fmt(st.alpha_out);
            });
            add_row("2SNS O", "Step 4", "t", [](const TimeStepSnapshot& st) { return fmt(st.t_out); });
            add_row("2SNS O", "Step 4", "message", [](const TimeStepSnapshot& st) {
                return st.time_header == "0/10000" ? "" : st.message;
            });

            return rows;
        }
    };

}