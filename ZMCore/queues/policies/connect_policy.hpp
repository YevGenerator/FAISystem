#pragma once
#include <zmq.hpp>

#include "DeviceConfig.hpp"
#include "../../IpCharAddress.hpp"
#include "../fixed_string.hpp"

namespace NodeSystem::ZMCore::Queues::Policies {
    template<typename T, typename... Args>
    concept IsConnectPolicy = requires(zmq::socket_t &socket, zmq::context_t &context, Args&&... args)
    {
        { T::init(socket, context, std::forward<Args>(args)...) } -> std::same_as<void>;
    };

    template<FixedString Address, int TimeoutMs = 500>
    struct ConnectPullPolicy {
        constexpr static auto address() -> decltype(Address) {
            return Address;
        }

        static void init(zmq::socket_t &socket, zmq::context_t &context) {
            socket = zmq::socket_t{context, zmq::socket_type::pull};
            socket.connect(Address);
            socket.set(zmq::sockopt::rcvtimeo, TimeoutMs);
        }
    };

    template<FixedString Address>
    struct BindPullPolicy {
        constexpr static auto address() -> decltype(Address) {
            return Address;
        }

        static void init(zmq::socket_t &socket, zmq::context_t &context) {
            socket = zmq::socket_t{context, zmq::socket_type::pull};
            socket.bind(Address);
        }
    };

    template<FixedString Address>
    struct ConnectPushPolicy {
        static void init(zmq::socket_t &socket, zmq::context_t &context) {
            socket = zmq::socket_t{context, zmq::socket_type::push};
            socket.bind(Address);
        }
    };

    struct ServerTcpConnectPolicy {
        static void init(zmq::socket_t &socket, zmq::context_t &context, const Core::Commands::KgIP &ip) {
            socket = zmq::socket_t{context, zmq::socket_type::router};
            socket.set(zmq::sockopt::router_mandatory, 1);
            const auto address = IpCharAddress<>::ipCharEmptyAddress(ip);
            socket.bind(address.c_str());
        }

        static void init(zmq::socket_t &socket, zmq::context_t &context, const Core::DeviceConfig &config) {
            init(socket, context, config.serverAddress);
        }
    };

    struct ClientTcpConnectPolicy {
        static void init(zmq::socket_t &socket, zmq::context_t &context, const Core::DeviceConfig& config) {
            socket = zmq::socket_t{context, zmq::socket_type::dealer};
            socket.set(zmq::sockopt::routing_id, zmq::buffer(&config.deviceId, sizeof(config.deviceId)));
            socket.set(zmq::sockopt::probe_router, 1);
            const auto address = IpCharAddress<>::ipCharAddress(config.serverAddress);
            socket.connect(address.c_str());
        }
    };

    static_assert(IsConnectPolicy<ServerTcpConnectPolicy, const Core::Commands::KgIP&>);
    static_assert(IsConnectPolicy<ClientTcpConnectPolicy, const Core::DeviceConfig&>);
    //using NodeProcessPacketPuller = PullerBase<Core::Commands::NodeProcessPacket>;
} // namespace NodeSystem::ZMCore::Queues::Policies
