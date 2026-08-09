#pragma once
#include <string_view>

namespace NodeSystem::Core::strings::Keywords {
    constexpr std::string_view Let = "let";
    constexpr std::string_view Device = "device";
    constexpr std::string_view Workers = "workers";
    constexpr std::string_view Ip = "ip";
    constexpr std::string_view Node = "node";
    constexpr std::string_view Sensor = "sensor";
    constexpr std::string_view Bind = "bind";
    constexpr std::string_view BindE = "bindE";
    constexpr std::string_view Run = "run";
    constexpr std::string_view Read = "read";

    namespace Params {
        constexpr std::string_view Device = "deviceId";
        constexpr std::string_view Id = "id";
        constexpr std::string_view Algo = "algo";
        constexpr std::string_view Inputs = "inputs";
        constexpr std::string_view Output = "output";
        constexpr std::string_view Host = "host";
        constexpr std::string_view Port = "port";
        constexpr std::string_view ToRun = "toRun";
        constexpr std::string_view In = "in";
        constexpr std::string_view To = "to";
        constexpr std::string_view Period = "period";
        constexpr std::string_view ToEmit = "toEmit";

        constexpr std::string_view OutC = "c";
        constexpr std::string_view OutA = "a";

        constexpr std::string_view InC = "c_i";
        constexpr std::string_view InA = "a_i";
        constexpr std::string_view InB = "b_i";
        constexpr std::string_view InV = "v_i";
        constexpr std::string_view InG = "g_i";
    }
}
