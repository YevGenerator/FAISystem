#pragma once

#include <span>
#include "NodeId.hpp"

namespace NodeSystem::Core::Nodes {
    struct ExternalAddress {
        types::ID deviceId{0};
    };

    struct InternalAddress {
        NodeId targetNodeId{};
        types::ID slotId{};
    };

    struct AddressView {
        std::span<const InternalAddress> internal;
        std::span<const ExternalAddress> external;
    };

    struct AddressList {
        AddressList() = default;

        AddressList(std::vector<InternalAddress> &&internal,
                    std::vector<ExternalAddress> &&external) : internalAddresses(internal),
                                                               externalAddresses(external) {
        }

        auto internal() -> auto & {
            return this->internalAddresses;
        }

        auto external() -> auto & {
            return this->externalAddresses;
        }

        auto view() -> AddressView {
            return {.internal=this->internalAddresses, .external=this->externalAddresses};
        }

    private:
        std::vector<InternalAddress> internalAddresses;
        std::vector<ExternalAddress> externalAddresses;
    };
} // namespace NodeSystem::Core::Nodes
