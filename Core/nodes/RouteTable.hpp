#pragma once

#include "NodeAddress.hpp"
#include "NodeMessage.hpp"
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace NodeSystem::Core::Nodes {
    template<typename TFunc, typename TArg>
    concept IsInvokable = std::invocable<TFunc, const TArg &>;

    class RouteTable {
    public:
        struct Entry {
            AddressView view;
            AddressList *address_ptr = nullptr;
        };

        std::unordered_map<NodeId, Entry> lookupRoutes;
        std::vector<std::unique_ptr<AddressList> > basement;

        void createAddresses(const NodeId id, std::vector<InternalAddress> &&internal = {},
                          std::vector<ExternalAddress> &&external = {}) {
            auto concrete = std::make_unique<AddressList>(std::move(internal), std::move(external));
            auto *raw_ptr = concrete.get();
            basement.push_back(std::move(concrete));
            lookupRoutes[id] = Entry{
                .view = concrete->view(),
                .address_ptr = raw_ptr,
            };
        }


        void add_address(NodeId id, const InternalAddress &internal) {
            auto it = lookupRoutes.find(id);
            if (it == lookupRoutes.end()) {
                return;
            }
            auto &entry = it->second;
            entry.address_ptr->internal().push_back(internal);
            entry.view = entry.address_ptr->view();
        }

        void add_address(NodeId id, const ExternalAddress &external) {
            auto it = lookupRoutes.find(id);
            if (it == lookupRoutes.end()) {
                return;
            }
            auto &entry = it->second;
            entry.address_ptr->external().push_back(external);
            entry.view = entry.address_ptr->view();
        }

        [[nodiscard]]
        auto view(const NodeId id) const -> AddressView {
            return lookupRoutes.at(id).view;
        }

        void route(const NodeResultMessage &resultMessage,
                   IsInvokable<NodeProcessMessage> auto &&internalCallback,
                   IsInvokable<NodeResultForwardMessage> auto &&externalCallback) {
            auto it = this->lookupRoutes.find(resultMessage.nodeId);
            if (it == this->lookupRoutes.end()) {
                return;
            }
            const auto &addresses = it->second.view;

            for (const auto &address: addresses.internal) {
                NodeProcessMessage processMessage{
                    .nodeId = address.targetNodeId,
                    .slotId = address.slotId,
                    .alpha = resultMessage.alpha,
                };
                internalCallback(processMessage);
            }

            for (auto address: addresses.external) {
                NodeResultForwardMessage transportMessage{
                    .deviceId = address.deviceId,
                    .nodeId = resultMessage.nodeId,
                    .alpha = resultMessage.alpha,
                };
                externalCallback(transportMessage);
            }
        }

    private:
        mutable std::shared_mutex routes_mutex{};
    };

} // namespace NodeSystem::Core::Nodes
