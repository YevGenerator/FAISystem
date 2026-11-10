#pragma once
#include <atomic>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include "LogRecordList.hpp"
namespace NodeSystem::Core {
struct FileWriteStream {
    std::ofstream out;
    void open(const std::filesystem::path& path) {
        out.close(); out.clear(); out.open(path, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("Unable to open log file: " + path.string());
    }
    template<typename Record>
    void write(const LogRecordList::LogRecord<Record>& record) {
        if (out.is_open()) { out.write(reinterpret_cast<const char*>(&record), sizeof(record)); out.flush(); }
    }
};
template<typename Writer = FileWriteStream, bool Enabled = true>
class Logger {
    Writer writer{};
    std::mutex logMutex;
    std::atomic<types::ID> deviceId{0};
public:
    void open(const std::filesystem::path& path) { std::scoped_lock lock{logMutex}; writer.open(path); }
    void setDeviceId(types::ID id) { deviceId.store(id, std::memory_order_relaxed); }
    [[nodiscard]] auto newEvent(const std::optional<Events::EventHeader>& parent = std::nullopt) const -> Events::EventHeader {
        if (parent && !parent->eventId.isNull())
            return Events::EventCounter::newEvent(deviceId.load(std::memory_order_relaxed), parent);
        return Events::EventCounter::newEvent(deviceId.load(std::memory_order_relaxed));
    }
    template<typename P>
    void log(const P& data, const Events::EventHeader& header) {
        if constexpr (Enabled) { std::scoped_lock lock{logMutex}; writer.write(LogRecordList::CreateLogRecord(data, header)); }
    }
};
inline Logger<> LoggerBin{};
}