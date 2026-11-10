#pragma once

#include "NodeAddress.hpp"
#include "NodeMessage.hpp"
#include <algorithm>
#include <concepts>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace NodeSystem::Core::Nodes {
    template<typename TFunc, typename TArg>
    concept IsInvokable = std::invocable<TFunc, const TArg &>;

    class RouteTable {
    public:
        struct Entry {
            AddressView view;
            AddressList* address_ptr = nullptr;
        };

        std::unordered_map<NodeId, Entry> lookupRoutes;
        std::vector<std::unique_ptr<AddressList>> basement;

        void createAddresses(NodeId id, std::vector<InternalAddress>&& internal = {},
                             std::vector<ExternalAddress>&& external = {}) {
            std::unique_lock lock(routes_mutex);
            createAddressesUnlocked(id, std::move(internal), std::move(external));
        }

        void add_address(NodeId id, const InternalAddress& internal) {
            std::unique_lock lock(routes_mutex);
            auto it = lookupRoutes.find(id);
            if (it == lookupRoutes.end()) {
                createAddressesUnlocked(id, {}, {});
                it = lookupRoutes.find(id);
            }
            auto& list = it->second.address_ptr->internal();
            if (std::find_if(list.begin(), list.end(), [&](const InternalAddress& a) {
                    return a.targetNodeId == internal.targetNodeId && a.slotId == internal.slotId;
                }) != list.end()) return;
            list.push_back(internal);
            it->second.view = it->second.address_ptr->view();
        }

        void add_address(NodeId id, const ExternalAddress& external) {
            std::unique_lock lock(routes_mutex);
            auto it = lookupRoutes.find(id);
            if (it == lookupRoutes.end()) {
                createAddressesUnlocked(id, {}, {});
                it = lookupRoutes.find(id);
            }
            auto& list = it->second.address_ptr->external();
            if (std::find_if(list.begin(), list.end(), [&](const ExternalAddress& a) {
                    return a.deviceId == external.deviceId;
                }) != list.end()) return;
            list.push_back(external);
            it->second.view = it->second.address_ptr->view();
        }

        [[nodiscard]] auto view(NodeId id) const -> AddressView {
            std::shared_lock lock(routes_mutex);
            return lookupRoutes.at(id).view;
        }

        void route(const NodeResultMessage& resultMessage,
                   IsInvokable<NodeProcessMessage> auto&& internalCallback,
                   IsInvokable<NodeResultForwardMessage> auto&& externalCallback) {
            std::shared_lock lock(routes_mutex);
            const auto it = lookupRoutes.find(resultMessage.nodeId);
            if (it == lookupRoutes.end()) return;
            const auto& addresses = it->second.view;

            for (const auto& address : addresses.internal) {
                internalCallback(NodeProcessMessage{
                    .nodeId = address.targetNodeId, .slotId = address.slotId,
                    .alpha = resultMessage.alpha
                });
            }
            for (const auto& address : addresses.external) {
                externalCallback(NodeResultForwardMessage{
                    .deviceId = address.deviceId, .nodeId = resultMessage.nodeId,
                    .alpha = resultMessage.alpha
                });
            }
        }

    private:
        void createAddressesUnlocked(NodeId id, std::vector<InternalAddress>&& internal,
                                     std::vector<ExternalAddress>&& external) {
            auto owner = std::make_unique<AddressList>(std::move(internal), std::move(external));
            auto* ptr = owner.get();
            basement.push_back(std::move(owner));
            lookupRoutes[id] = Entry{.view = ptr->view(), .address_ptr = ptr};
        }

        mutable std::shared_mutex routes_mutex{};
    };
}
