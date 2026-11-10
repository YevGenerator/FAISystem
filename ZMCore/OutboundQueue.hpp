#pragma once
#include <cstring>
#include <deque>
#include <mutex>
#include <type_traits>
#include <vector>
#include "types/basic.hpp"

namespace NodeSystem::ZMCore {
    struct OutboundMessage {
        Core::types::ID deviceId{};
        std::vector<unsigned char> bytes;
    };


    class OutboundQueue {
    public:
        template<typename Packet>
        void push(Core::types::ID recipient, const Packet& packet) {
            static_assert(std::is_trivially_copyable_v<Packet>);
            OutboundMessage outgoing{recipient, std::vector<unsigned char>(sizeof(Packet))};
            std::memcpy(outgoing.bytes.data(), &packet, sizeof(Packet));
            std::scoped_lock lock(mutex_);
            messages_.push_back(std::move(outgoing));
        }

        std::deque<OutboundMessage> drain() {
            std::scoped_lock lock(mutex_);
            std::deque<OutboundMessage> result;
            result.swap(messages_);
            return result;
        }

    private:
        std::mutex mutex_;
        std::deque<OutboundMessage> messages_;
    };
}
