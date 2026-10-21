#pragma once
#include <fstream>
#include <iomanip>
#include <sstream>
#include "table_writer.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {

    class TextWriter : public ITableWriter {
        std::ofstream m_out;
        static constexpr int ColWidth = 11;

        void printSeparator(std::size_t count) {
            m_out << "+";
            for (std::size_t i = 0; i < count; ++i) {
                m_out << std::string(ColWidth + 2, '-') << "+";
            }
            m_out << "\n";
        }

    public:
        explicit TextWriter(const std::string& filepath)
            : m_out(filepath) {
            m_out << std::setprecision(4) << std::fixed;
        }

        void writeHeader() override {
            const auto& cols = AlgoTraceRecord::headers();
            printSeparator(cols.size());

            m_out << "|";
            for (const auto& col : cols) {
                m_out << " " << std::setw(ColWidth) << col << " |";
            }
            m_out << "\n";

            printSeparator(cols.size());
        }

        void writeRow(const AlgoTraceRecord& r) override {
            m_out << "| " << std::setw(ColWidth) << r.message_index
                  << " | " << std::setw(ColWidth) << r.time_ns
                  << " | " << std::setw(ColWidth) << r.input_alpha
                  << " | " << std::setw(ColWidth) << r.slot_index
                  << " | " << std::setw(ColWidth) << r.theta_i
                  << " | " << std::setw(ColWidth) << r.tau_i
                  << " | " << std::setw(ColWidth) << r.slot_alpha
                  << " | " << std::setw(ColWidth) << (r.slot_c ? "true" : "false")
                  << " | " << std::setw(ColWidth) << r.slot_a
                  << " | " << std::setw(ColWidth) << r.slot_g
                  << " | " << std::setw(ColWidth) << r.beta_i
                  << " | " << std::setw(ColWidth) << r.beta_norm
                  << " | " << std::setw(ColWidth) << r.m
                  << " | " << std::setw(ColWidth) << r.v_beta
                  << " | " << std::setw(ColWidth) << r.k
                  << " | " << std::setw(ColWidth) << r.cf_i
                  << " | " << std::setw(ColWidth) << r.ro_i
                  << " | " << std::setw(ColWidth) << r.contribution
                  << " | " << std::setw(ColWidth) << r.current_cf_star
                  << " | " << std::setw(ColWidth) << r.final_alpha
                  << " | " << std::setw(ColWidth) << (r.state_switched ? "YES" : "NO")
                  << " | " << std::setw(ColWidth) << (r.current_node_state ? "1" : "0")
                  << " |\n";
        }

        void flush() override {
            const auto& cols = AlgoTraceRecord::headers();
            printSeparator(cols.size());
            m_out.flush();
        }
    };

} // namespace NodeSystem::Testing