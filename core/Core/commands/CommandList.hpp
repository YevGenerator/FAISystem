#pragma once
#include <array>
#include <cstring>
#include <optional>
#include <variant>

#include "CmdTypes.hpp"
#include "NetworkPacket.hpp"
#include "events/EventHeader.hpp"
#include "nodes/NodeMessage.hpp"

namespace NodeSystem::Core::Commands {
    template<typename... T>
    struct CommandRegistry {
        static constexpr types::Byte count = sizeof ...(T);

        template<typename K>
        static constexpr types::Byte id = [] {
            types::Byte index = 0;
            auto found = ((std::is_same_v<K, T> ? true : (++index, false)) || ...);
            return index;
        }();

        template<typename TContent>
        static constexpr auto CreatePacket(TContent &&content, Events::EventHeader header = {}) {
            using CleanContent = std::decay_t<TContent>;

            return NetworkPacket<CleanContent>{
                .commandId = id<CleanContent>,
                .eventHeader = header,
                .data = std::forward<TContent>(content),
            };
        }

        using Variant = std::variant<std::monostate, T...>;

        template<typename P>
        static auto readMessageNetwork(const char *buffer) -> NetworkPacket<Variant> {
            NetworkPacket<Variant> msg{};
            constexpr std::size_t headerSize = NetworkPacket<P>::headerSize();
            std::memcpy(&msg, buffer, headerSize);
            P specificData{};
            std::memcpy(&specificData, buffer + headerSize, sizeof(P));
            msg.data = specificData;

            return msg;
        }

        template<typename P>
        static auto readMessageFile(const char *&buffer, std::size_t &available) -> Variant {
            if (available < sizeof(P)) {
                return std::monostate{};
            }

            P specificData{};
            std::memcpy(&specificData, buffer, sizeof(P));
            buffer += sizeof(P);
            available -= sizeof(P);
            return specificData;
        }


        static constexpr auto handlersNetwork() {
            return std::array{&readMessageNetwork<T>...};
        };

        static constexpr auto handlersFile() {
            return std::array{&readMessageFile<T>...};
        }

        static auto nextFromFile(const char *&buffer, std::size_t &available) -> std::optional<Variant> {
            if (available == 0) {
                return std::nullopt; // EOF
            }

            auto opcode = static_cast<types::Byte>(*buffer);
            buffer++;
            available--;

            constexpr auto handlers = handlersFile();
            if (opcode < count) {
                return handlers[opcode](buffer, available);
            }
            return Variant{std::monostate{}};
        }
    };

    using CommandList = CommandRegistry<
        CmdDevice,
        CmdWorkers,
        CmdIp,
        CmdSensorCreate,
        CmdRun,
        CmdRead,
        CmdNodeCreate,
        CmdNodeAddInput,
        CmdBindDouble,
        CmdBindSingle,
        Nodes::NodeProcessMessage,
        Nodes::NodeResultMessage,
        Nodes::NodeResultForwardMessage
    >;

    using NodeResultPacket = NetworkPacket<Nodes::NodeResultMessage>;
    using NodeProcessPacket = NetworkPacket<Nodes::NodeProcessMessage>;
    using NodeTransportPacket = NetworkPacket<Nodes::NodeResultForwardMessage>;
} // namespace NodeSystem::Core::Commands
