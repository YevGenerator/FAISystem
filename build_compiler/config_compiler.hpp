#pragma once
#include <format>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <map>
#include <regex>
#include <variant>

#include "ConfigReader.hpp"
#include "../strings/cmd_keywords.hpp"
#include "../strings/CommandParser.hpp"
#include "../strings/Utils.hpp"
#include "../coreі/types.hpp"
#include "../coreі/commands/CommandList.hpp"

namespace NodeSystem::Compiler {
    namespace s = Core::strings;
    namespace t = Core::types;
    class ConfigCompiler {

        template<typename Cmd>
        static void writeCmd(std::ofstream &out, const Cmd &cmd) {
            const t::Byte op = Core::CommandList::id<Cmd>;
            out.write(reinterpret_cast<const char *>(&op), 1);
            out.write(reinterpret_cast<const char *>(&cmd), sizeof(Cmd));
        }
    public:
        static void compile(const std::string &inFile, const std::string &outFile) {
            std::ifstream in(inFile);
            std::ofstream out(outFile, std::ios::binary);

            if (!in || !out) {
                std::cerr << "File access error.\n";
                return;
            }

            Core::FileHeader header{};
            out.write(reinterpret_cast<const char *>(&header), sizeof(header));

            std::string fullText, line;
            while (std::getline(in, line)) {
                line = s::CommandParser::trim(line);
                if (!line.empty() && line[0] != '#') fullText += line + " ";
            }

            std::map<std::string, std::string> env;
            Core::types::ID currentDevice = 0;

            namespace kw = s::Keywords;
            auto row =
                    {kw::Let, kw::Device, kw::Workers, kw::Ip, kw::Node, kw::Sensor, kw::BindE, kw::Bind, kw::Run, kw::Read};
            std::regex cmdRegex(std::format(R"(\b({})\b)", s::Utils::join(row, "|")));
            auto words_begin = std::sregex_iterator(fullText.begin(), fullText.end(), cmdRegex);
            auto words_end = std::sregex_iterator();

            std::vector<std::string> rawCommands;
            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::sregex_iterator next = i;
                ++next;
                std::size_t start = i->position();
                std::size_t end = (next != words_end) ? next->position() : fullText.length();
                rawCommands.push_back(s::CommandParser::trim(fullText.substr(start, end - start)));
            }

            for (const auto &rawCmd: rawCommands) {
                auto tokens = s::CommandParser::splitTokens(rawCmd);
                if (tokens.empty()) continue;

                if (tokens[0] == kw::Let && tokens.size() >= 4) {
                    const std::string& name = tokens[2];
                    std::string val = rawCmd.substr(rawCmd.find('=') + 1);
                    env[name] = s::CommandParser::trim(val);
                    continue;
                }

                auto variants = s::CommandParser::parseCommand(rawCmd, env, currentDevice);

                for (const auto &var: variants) {
                    std::visit([&out](const auto &cmd) {
                        using T = std::decay_t<decltype(cmd)>;
                        if constexpr (!std::is_same_v<T, std::monostate>) {
                            writeCmd(out, cmd);
                        }
                    }, var);
                }
            }
        }

    };
}
