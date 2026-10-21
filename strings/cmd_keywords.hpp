#pragma once
#include <string_view>

namespace NodeSystem::strings::Keywords
{
    namespace Cmd
    {
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
    }


    namespace Chars
    {
        constexpr std::string_view Comment = "#";
        constexpr std::string_view StructOpen = "<";
        constexpr std::string_view StructClose = ">";
        constexpr std::string_view ListStart = "[";
        constexpr std::string_view ListEnd = "]";
        constexpr std::string_view AssignParam = "=";
        constexpr std::string_view AssignVar = " = ";
    }

    namespace Params
    {
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
        constexpr std::string_view From = "from";
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

    namespace Types
    {
        constexpr std::string_view Uint32 = "uint32";
        constexpr std::string_view Uint16 = "uint16";
        constexpr std::string_view Uint8 = "uint8";
        constexpr std::string_view Float = "Float";
        constexpr std::string_view NodeId = "KgNodeId";
        constexpr std::string_view Direction = "Direction";
        constexpr std::string_view Algo = "AlgoType";
        constexpr std::string_view KgNode = "KgNode";
        constexpr std::string_view KgInput = "KgInput";
        constexpr std::string_view KgOutput = "KgOutput";
        constexpr std::string_view KgSensor = "KgSensor";
        constexpr std::string_view KgIp = "KgIp";
        constexpr std::string_view KgRun = "KgRun";
        constexpr std::string_view KgBindFrom = "KgBindFrom";
        constexpr std::string_view KgBindTo = "KgBindTo";
        constexpr std::string_view KgBindSingle = "KgBindSingle";
        constexpr std::string_view KgBindDouble = "KgBindDouble";
    }
} // namespace NodeSystem::strings::Keywords
