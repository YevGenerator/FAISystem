#pragma once
#include <fstream>
#include <iomanip>
#include "table_writer.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {

    class CsvWriter : public ITableWriter {
        std::ofstream m_out;
        char m_delimiter;

    public:
        explicit CsvWriter(const std::string& filepath, char delimiter = ',')
            : m_out(filepath), m_delimiter(delimiter) {
            m_out << std::setprecision(6) << std::fixed;
        }

        void writeHeader() override {
            const auto& cols = AlgoTraceRecord::headers();
            for (std::size_t i = 0; i < cols.size(); ++i) {
                m_out << cols[i] << (i + 1 < cols.size() ? m_delimiter : '\n');
            }
        }

        void writeRow(const AlgoTraceRecord& r) override {
            m_out << r.message_index << m_delimiter
                  << r.time_ns << m_delimiter
                  << r.input_alpha << m_delimiter
                  << r.slot_index << m_delimiter
                  << r.theta_i << m_delimiter
                  << r.tau_i << m_delimiter
                  << r.slot_alpha << m_delimiter
                  << (r.slot_c ? 1 : 0) << m_delimiter
                  << r.slot_a << m_delimiter
                  << r.slot_g << m_delimiter
                  << r.beta_i << m_delimiter
                  << r.beta_norm << m_delimiter
                  << r.m << m_delimiter
                  << r.v_beta << m_delimiter
                  << r.k << m_delimiter
                  << r.cf_i << m_delimiter
                  << r.ro_i << m_delimiter
                  << r.contribution << m_delimiter
                  << r.current_cf_star << m_delimiter
                  << r.final_alpha << m_delimiter
                  << (r.state_switched ? 1 : 0) << m_delimiter
                  << (r.current_node_state ? 1 : 0) << '\n';
        }

        void flush() override {
            m_out.flush();
        }
    };

} // namespace NodeSystem::Testing