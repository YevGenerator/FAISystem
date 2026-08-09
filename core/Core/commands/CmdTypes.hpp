#pragma once

#include <limits>

#include "../types/basic.hpp"
#include "types/NetField.hpp"
#include "../nodes/NodeInfo.hpp"

namespace NodeSystem::Core::Commands {
#pragma pack(push, 1)
    struct KgBindFrom {
        types::ID device;
        types::ID id_lvl;
        types::ID id_val;
    };

    struct KgBindTo {
        types::ID device;
        types::ID id_lvl;
        types::ID id_val;
        types::ID in_index;
    };

    struct KgIP {
        std::uint32_t host{};
        types::PortInt port{};
    };

    struct CmdDevice {
        types::ID deviceId;
    };

    struct CmdWorkers {
        types::WorkerInt workerAmount;
    };

    struct CmdIp: KgIP {
    };

    struct CmdRun {
        types::ID device{};
        types::NetBool toRun;
    };

    struct CmdSensorCreate {
        types::ID device{};
        types::ID id{};
        types::ID pin_index{};
    };

    struct CmdRead {
        std::array<char, std::numeric_limits<types::Byte>::max()> filename;
    };

    struct CmdNodeCreate {
        types::ID device{};
        types::ID id_lvl{};
        types::ID id_val{};
        types::Byte algo{0};
        types::NetBool output_c;
        types::NetDouble output_a;
        types::ID inputs_count{};

        [[nodiscard]]
        auto toNodeCreateInfo() const -> Nodes::NodeCreateInfo {
            return {
                .id = {.level = id_lvl, .index = id_val},
                .algoType = algo,
                .output = {.c = output_c, .a = output_a},
            };
        }
    };

    struct CmdNodeAddInput {
        types::ID device{};
        types::ID id_lvl{};
        types::ID id_val{};
        types::ID input_index{};
        Nodes::KgInput input_data;
    };

    struct CmdBindDouble {
        KgBindFrom from;
        KgBindTo to;
    };

    struct CmdBindSingle {
        KgBindFrom from;
        types::ID deviceTo;
    };

#pragma pack(pop)
} // namespace NodeSystem::Core::Commands
