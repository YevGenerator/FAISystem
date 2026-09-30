#pragma once
#include <ostream>

#include "cmd_keywords.hpp"
#include "convert/all_converters.hpp"
#include "dto/dto.hpp"

namespace NodeSystem::Compiler
{
    struct TextWriter
    {
        std::ostream& out;

        void operator()(const strings::dto::Id& deviceId) const
        {
            out << strings::Keywords::Cmd::Device << " " << deviceId << "\n";
        }

        void operator()(const strings::dto::KgIp& ip) const
        {
            out << strings::Keywords::Cmd::Ip << " "
                << strings::convert::KgIpConverter::toString(ip) << "\n";
        }

        void operator()(const strings::dto::KgSensor& sensor) const
        {
            out << strings::Keywords::Cmd::Sensor << " "
                << strings::convert::KgSensorConverter::toString(sensor) << "\n";
        }

        void operator()(const strings::dto::KgRun& run) const
        {
            out << strings::Keywords::Cmd::Run << " "
                << strings::convert::KgRunConverter::toString(run) << "\n";
        }

        void operator()(const strings::dto::KgBindSingle& bind) const
        {
            out << strings::Keywords::Cmd::Bind << " "
                << strings::convert::KgBindSingleConverter::toString(bind) << "\n";
        }

        void operator()(const strings::dto::KgBindDouble& bind) const
        {
            out << strings::Keywords::Cmd::BindE << " "
                << strings::convert::KgBindDoubleConverter::toString(bind) << "\n";
        }

        void operator()(const strings::dto::KgNode& node) const
        {
            out << strings::Keywords::Cmd::Node << " "
                << strings::convert::KgNodeConverter::toString(node) << "\n";
        }
    };
}