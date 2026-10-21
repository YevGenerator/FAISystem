#pragma once
#include <memory>
#include <vector>
#include "../algo_trace_record.hpp"

namespace NodeSystem::Testing::Calculate::twoSNO {
    class ITableWriter {
    public:
        virtual ~ITableWriter() = default;
        virtual void writeHeader() = 0;
        virtual void writeRow(const AlgoTraceRecord& record) = 0;
        virtual void flush() = 0;
    };

    // Зручний композит, щоб писати в 3 формати одночасно
    class CompositeTableWriter : public ITableWriter {
        std::vector<std::unique_ptr<ITableWriter>> m_writers;
    public:
        void addWriter(std::unique_ptr<ITableWriter> writer) {
            m_writers.push_back(std::move(writer));
        }

        void writeHeader() override {
            for (auto& w : m_writers) w->writeHeader();
        }

        void writeRow(const AlgoTraceRecord& record) override {
            for (auto& w : m_writers) w->writeRow(record);
        }

        void flush() override {
            for (auto& w : m_writers) w->flush();
        }
    };

} // namespace NodeSystem::Testing