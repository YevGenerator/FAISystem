#pragma once
#include <cstdint>

namespace NodeSystem::Core::types {
    using Byte = std::uint8_t;
    using ID = std::uint32_t;
    using IP = std::uint32_t;
    using PortInt = std::uint16_t;
    using WorkerInt = std::uint8_t;
    using MsInt = std::uint64_t;
    using UsInt = std::uint64_t;
    using EventIDInt = std::uint64_t;
    using SeedInt = std::uint64_t;

    using Float = double;


} // namespace NodeSystem::Core::types
