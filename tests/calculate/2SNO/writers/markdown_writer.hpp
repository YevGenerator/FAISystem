#pragma once
#include <fstream>
#include <iomanip>
#include "table_writer.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {

    class MarkdownWriter : public ITableWriter {
        std::ofstream m_out;

    public:
        explicit MarkdownWriter(const std::string& filepath)
            : m_out(filepath) {
            m_out << std::setprecision(4) << std::fixed;
        }

        void writeHeader() override {
            const auto& cols = AlgoTraceRecord::headers();

            // Заголовки
            m_out << "|";
            for (const auto& col : cols) {
                m_out << " " << col << " |";
            }
            m_out << "\n|";

            // Розділювач колонок Markdown (:---:)
            for (std::size_t i = 0; i < cols.size(); ++i) {
                m_out << ":---:|";
            }
            m_out << "\n";
        }

        void writeRow(const AlgoTraceRecord& r) override {
            m_out << "| " << r.message_index
                  << " | " << r.time_ns
                  << " | " << r.input_alpha
                  << " | " << r.slot_index
                  << " | " << r.theta_i
                  << " | " << r.tau_i
                  << " | " << r.slot_alpha
                  << " | " << (r.slot_c ? "true" : "false")
                  << " | " << r.slot_a
                  << " | " << r.slot_g
                  << " | " << r.beta_i
                  << " | " << r.beta_norm
                  << " | " << r.m
                  << " | " << r.v_beta
                  << " | " << r.k
                  << " | " << r.cf_i
                  << " | " << r.ro_i
                  << " | " << r.contribution
                  << " | " << r.current_cf_star
                  << " | " << r.final_alpha
                  << " | " << (r.state_switched ? "YES" : "NO")
                  << " | " << (r.current_node_state ? "1" : "0")
                  << " |\n";
        }

        void flush() override {
            m_out.flush();
        }
    };

} // namespace NodeSystem::Testing