#pragma once

namespace NodeSystem::Core::types {
    template<class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    overload(Ts...) -> overload<Ts...>;
} // namespace NodeSystem::Core::types
