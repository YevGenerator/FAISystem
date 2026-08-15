#pragma once

#include <optional>
#include <vector>

#include "../Core/types/ConfigTypes.hpp"
#include "../Core/commands/CmdTypes.hpp"
#include "../strings/CommandParser.hpp"
#include <fstream>

#include "ConfigReader.hpp"
#include "../Core/types/overload.hpp"
#include "../strings/Utils.hpp"

namespace NodeSystem::Compiler {
    namespace t = Core::types;
    namespace kw = Core::strings::Keywords;
    namespace prm = Core::strings::Keywords::Params;

    struct NodeBuilder {
        t::ConfigCmdNodeCreate createCmd;
        std::vector<t::ConfigCmdNodeAddInput> inputs;
    };

    struct BindBuilder {
        t::KgBindFrom source;
        std::vector<t::KgBindTo> targets;
    };

    class ConfigDecompiler {
        inline static std::optional<NodeBuilder> pendingNode;
        inline static std::optional<BindBuilder> pendingBind;


        static void flushNode(std::ofstream &out) {
            if (!pendingNode) return;
            auto &n = pendingNode->createCmd;

            std::string_view algoName = Core::strings::AlgoNames::algoName(n.algo);
            out << kw::Node << " <"
                    << prm::Device << "=" << n.device << " "
                    << prm::Id << "=" << n.id_lvl << "." << n.id_val << " "
                    << prm::Algo << "=\"" << algoName << "\" "
                    << prm::Output << "=<"
                    << prm::OutC << "=\"" << (n.output.c ? "up" : "down") << "\" "
                    << prm::OutA << "=" << n.output.a << ">"; // ТУТ ТЕПЕР ОДНА ДУЖКА >

            if (!pendingNode->inputs.empty()) {
                out << " " << prm::Inputs << "=[ ";
                for (const auto &in: pendingNode->inputs) {
                    out << "<"
                            << prm::InC << "=\"" << (in.input_data.c_i ? "up" : "down") << "\" "
                            << prm::InA << "=" << in.input_data.a_i << " "
                            << prm::InB << "=" << in.input_data.b_i << " "
                            << prm::InV << "=" << in.input_data.v_i << " "
                            << prm::InG << "=" << in.input_data.g_i << "> ";
                }
                out << "]";
            }
            out << ">\n";
            pendingNode.reset();
        }

        static void flushBind(std::ofstream &out) {
            if (!pendingBind) return;
            auto &src = pendingBind->source;

            // Новий синтаксис: bind <deviceId=... id=... to=[...]>
            out << kw::Bind << " <"
                    << prm::Device << "=" << src.device << " "
                    << prm::Id << "=" << src.id_lvl << "." << src.id_val;

            if (!pendingBind->targets.empty()) {
                out << " " << prm::To << "=[ ";
                for (const auto &tgt: pendingBind->targets) {
                    out << "<"
                            << prm::Device << "=" << tgt.device << " "
                            << prm::Id << "=" << tgt.id_lvl << "." << tgt.id_val << " "
                            << prm::In << "=" << tgt.in_index << "> ";
                }
                out << "]";
            }
            out << ">\n";
            pendingBind.reset();
        }

        static void flushAll(std::ofstream &out) {
            flushNode(out);
            flushBind(out);
        }

    public:
        static void decompile(const std::string &inFile, const std::string &outFile) {
            std::ifstream in(inFile, std::ios::binary | std::ios::ate);
            if (!in) {
                std::cerr << "Cannot open input file: " << inFile << "\n";
                return;
            }

            std::size_t size = in.tellg();
            in.seekg(0);
            std::vector<char> buffer(size);
            in.read(buffer.data(), size);

            if (size < sizeof(Core::FileHeader)) {
                std::cerr << "File is too small to be a valid config.\n";
                return;
            }

            auto *header = reinterpret_cast<Core::FileHeader *>(buffer.data());
            if (header->magic != Core::MAGIC_BYTES || header->version != Core::FORMAT_VERSION) {
                std::cerr << "Invalid format or version mismatch!\n";
                return;
            }

            std::ofstream out(outFile);
            out << "# Decompiled RPConfig File v" << header->version << "\n\n";

            const char *ptr = buffer.data() + sizeof(Core::FileHeader);
            std::size_t available = size - sizeof(Core::FileHeader);

            while (available > 0) {
                auto variantOpt = Core::CommandList::nextFromFile(ptr, available);
                if (!variantOpt) break;

                std::visit(overload{
                               [&](std::monostate) {
                               },
                               [&](const Core::types::ConfigCmdDevice &cmd) {
                                   flushAll(out);
                                   out << kw::Device << " " << cmd.deviceId << "\n";
                               },
                               [&](const Core::types::ConfigCmdWorkers &cmd) {
                                   flushAll(out);
                                   out << kw::Workers << " " << static_cast<int>(cmd.workerAmount) << "\n";
                               },
                               [&](const Core::types::ConfigCmdIp &cmd) {
                                   flushAll(out);
                                   out << kw::Ip << " <"
                                           << prm::Host << "=" << Core::strings::Utils::ip_to_string(cmd.ipData.host) <<
                                           " "
                                           << prm::Port << "=" << cmd.ipData.port << ">\n";
                               },
                               [&](const Core::types::ConfigCmdRun &cmd) {
                                   flushAll(out);
                                   out << kw::Run << " <"
                                           << prm::Device << "=" << cmd.device << " "
                                           << prm::ToRun << "=" << (cmd.toRun ? "1" : "0") << ">\n";
                               },
                               [&](const Core::types::ConfigCmdSensorCreate &cmd) {
                                   flushAll(out);
                                   out << kw::Sensor << " <"
                                           << prm::Device << "=" << cmd.device << " "
                                           << prm::Id << "=" << cmd.id_val << " "
                                           << prm::Period << "=" << cmd.period << " "
                                           << prm::ToEmit << "=" << cmd.valueToEmit << ">\n";
                               },
                               [&](const Core::types::ConfigCmdRead &cmd) {
                                   flushAll(out);
                                   out << kw::Read << " \"" << cmd.filename << "\"\n";
                               },
                               [&](const Core::types::ConfigCmdNodeCreate &cmd) {
                                   flushAll(out);
                                   pendingNode = NodeBuilder{cmd, {}};
                                   if (cmd.inputs_count == 0) flushNode(out);
                               },
                               [&](const Core::types::ConfigCmdNodeAddInput &cmd) {
                                   flushBind(out);
                                   if (pendingNode &&
                                       pendingNode->createCmd.device == cmd.device &&
                                       pendingNode->createCmd.id_lvl == cmd.id_lvl &&
                                       pendingNode->createCmd.id_val == cmd.id_val) {
                                       pendingNode->inputs.push_back(cmd);
                                       if (pendingNode->inputs.size() == pendingNode->createCmd.inputs_count) {
                                           flushNode(out);
                                       }
                                   }
                               },
                               [&](const Core::types::ConfigCmdBindDouble &cmd) {
                                   flushNode(out);
                                   if (pendingBind && (
                                           pendingBind->source.device != cmd.from.device ||
                                           pendingBind->source.id_lvl != cmd.from.id_lvl ||
                                           pendingBind->source.id_val != cmd.from.id_val)) {
                                       flushBind(out);
                                   }
                                   if (!pendingBind) pendingBind = BindBuilder{cmd.from, {}};
                                   pendingBind->targets.push_back(cmd.to);
                               },
                               [&](const Core::types::ConfigCmdBindSingle &cmd) {
                                   flushAll(out);
                                   out << kw::BindE << " <"
                                           << prm::Device << "=" << cmd.from.device << " "
                                           << prm::Id << "=" << cmd.from.id_lvl << "." << cmd.from.id_val << " "
                                           << prm::To << "=" << cmd.deviceTo << ">\n";
                               },
                               [&](const auto &) {
                               }
                           }, *variantOpt);
            }

            flushAll(out);
        }
    };
}
