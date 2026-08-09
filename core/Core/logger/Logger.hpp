#pragma once
#include <mutex>
#include <fstream>
#include "LogRecordList.hpp"

namespace NodeSystem::Core {
    template<typename Writer, typename P>
    concept CanWriteRecord = requires(Writer w, const LogRecordList::LogRecord<P> &r)
    {
        { w.write(r) } -> std::same_as<void>;
    };

    template<typename Writer, typename... Ts>
    concept IsLogWriter = (CanWriteRecord<Writer, Ts> && ...);

    struct FileWriteStream {
        std::ofstream out;

        explicit FileWriteStream(const std::string &path)
            : out(path, std::ios::binary) {
        }

        template<typename Record>
        void write(const LogRecordList::LogRecord<Record> &record) {
            out.write(reinterpret_cast<const char *>(&record), sizeof(record));
        }
    };

    static_assert(IsLogWriter<FileWriteStream>);


    template<typename Writer, bool Enabled = true>
        requires IsLogWriter<Writer>
    struct Logger {
    private:
        Writer writer;
        inline static std::mutex log_mutex{};

    public:
        explicit Logger(Writer w) : writer(std::move(w)) {
        }

        template<typename P>
        void log(const P &record, const EventCounter::EventHeader &header = EventCounter::event()) {
            if constexpr (Enabled) {
                std::scoped_lock lock{log_mutex};
                writer.write(LogRecordList::CreateLogRecord<P>(record, header));
            }
        }
    };

    inline Logger<FileWriteStream, true> LoggerBin{FileWriteStream{"log2.bin"}};
}
