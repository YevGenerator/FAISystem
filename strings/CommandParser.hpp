#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <iostream>
#include <cstring>
#include <functional>

#include "algoname_map.hpp"
#include "cmd_keywords.hpp"
#include "../coreі/commands/CommandList.hpp"
#include "types.hpp"


namespace NodeSystem::Core::strings {

    using ParamMap = std::map<std::string, std::string, std::less<>>;

    class CommandParser {
        static constexpr auto trimmedChars = " \t\r\n";
    private:
        static const std::string* getParam(const ParamMap& map, std::string_view key) {
            if (auto it = map.find(key); it != map.end()) {
                return &it->second;
            }
            return nullptr;
        }

    public:
        static std::string trim(const std::string& s) {
            const auto start = s.find_first_not_of(trimmedChars);
            if (start == std::string::npos) return "";
            const auto end = s.find_last_not_of(trimmedChars);
            return s.substr(start, end - start + 1);
        }

        static std::vector<std::string> splitTokens(const std::string& text) {
            std::vector<std::string> tokens;
            std::string current;
            int depth = 0; bool in_quotes = false;
            for (char c : text) {
                if (c == '"') { in_quotes = !in_quotes; current += c; }
                else if ((c == '<' || c == '[') && !in_quotes) { depth++; current += c; }
                else if ((c == '>' || c == ']') && !in_quotes) { depth--; current += c; }
                else if (std::isspace(c) && depth == 0 && !in_quotes) {
                    if (!current.empty()) { tokens.push_back(current); current.clear(); }
                } else { current += c; }
            }
            if (!current.empty()) tokens.push_back(current);
            return tokens;
        }

        static void parseId(const std::string& str, types::ID& lvl, types::ID& val) {
            auto pos = str.find('.');
            if (pos != std::string::npos) {
                lvl = std::stoul(str.substr(0, pos));
                val = std::stoul(str.substr(pos + 1));
            } else { lvl = 0; val = std::stoul(str); }
        }

        static std::uint32_t parseHost(const std::string& str) {
            std::uint32_t a = 0, b = 0, c = 0, d = 0;
            if (sscanf(str.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
                return a | (b << 8) | (c << 16) | (d << 24);
            return 0;
        }

        static types::SmallInt parseC(const std::string& str) {
            return (str.find("up") != std::string::npos || str.find("Up") != std::string::npos) ? 1 : 0;
        }

        static types::SmallInt parseBoolStrict(const std::string& str) {
            return (str == "1" || str == "true" || parseC(str) == 1) ? 1 : 0;
        }

        static ParamMap parseStructContent(
            std::string content,
            const std::map<std::string, std::string>& env,
            const std::vector<std::string_view>& fieldNames)
        {
            content = trim(content);
            if (!content.empty() && content.front() == '<') content = content.substr(1, content.length() - 2);

            ParamMap result;
            auto tokens = splitTokens(content);
            int posIndex = 0;

            for (const auto& tok : tokens) {
                auto eq = tok.find('=');
                if (eq != std::string::npos && tok.front() != '<' && tok.front() != '[') {
                    std::string key = tok.substr(0, eq);
                    std::string val = tok.substr(eq + 1);
                    auto envIt = env.find(val);
                    result[key] = (envIt != env.end()) ? envIt->second : val;
                } else {
                    while (posIndex < fieldNames.size() && result.contains(fieldNames[posIndex])) posIndex++;
                    if (posIndex < fieldNames.size()) {
                        auto envIt = env.find(tok);
                        result[std::string(fieldNames[posIndex])] = (envIt != env.end()) ? envIt->second : tok;
                        posIndex++;
                    }
                }
            }
            return result;
        }

        // --- УНІВЕРСАЛЬНИЙ ПАРСЕР ОДНІЄЇ КОМАНДИ ---
        static std::vector<CommandList::Variant> parseCommand(
            const std::string& rawCmd,
            const std::map<std::string, std::string>& env,
            types::ID& currentDevice)
        {
            std::vector<CommandList::Variant> generated;
            auto tokens = splitTokens(rawCmd);
            if (tokens.empty()) return generated;

            const std::string& cmd = tokens[0];
            using namespace types;

            if (cmd == Keywords::Device) {
                std::string val = tokens[1];
                if (auto it = env.find(val); it != env.end()) val = it->second;

                currentDevice = std::stoul(val);
                generated.emplace_back(ConfigCmdDevice{currentDevice});
            }
            else if (cmd == Keywords::Workers) {
                std::string val = tokens[1];
                if (auto it = env.find(val); it != env.end()) val = it->second;
                generated.emplace_back(ConfigCmdWorkers{static_cast<SmallInt>(std::stoul(val))});
            }
            else if (cmd == Keywords::Ip) {
                std::string val = trim(rawCmd.substr(Keywords::Ip.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                auto f = parseStructContent(val, env, {Keywords::Params::Host, Keywords::Params::Port});

                ConfigCmdIp ipCmd{};
                if (auto p = getParam(f, Keywords::Params::Host)) ipCmd.ipData.host = parseHost(*p);
                if (auto p = getParam(f, Keywords::Params::Port)) ipCmd.ipData.port = std::stoul(*p);
                generated.emplace_back(ipCmd);
            }
            else if (cmd == Keywords::Run) {
                std::string val = trim(rawCmd.substr(Keywords::Run.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                auto f = parseStructContent(val, env, {Keywords::Params::Device, Keywords::Params::ToRun});

                ConfigCmdRun runCmd{};
                runCmd.device = currentDevice;
                if (auto p = getParam(f, Keywords::Params::Device)) runCmd.device = std::stoul(*p);
                if (auto p = getParam(f, Keywords::Params::ToRun))  runCmd.toRun = parseBoolStrict(*p);
                generated.emplace_back(runCmd);
            }
            else if (cmd == Keywords::Sensor) {
                std::string val = trim(rawCmd.substr(Keywords::Sensor.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;

                // 1. Додаємо нові параметри у список для парсингу
                auto f = parseStructContent(val, env, {Keywords::Params::Device, Keywords::Params::Id, Keywords::Params::Period, Keywords::Params::ToEmit});

                ConfigCmdSensorCreate sensCmd{};
                sensCmd.device = currentDevice;
                types::ID ignoreLvl;

                if (auto p = getParam(f, Keywords::Params::Device)) sensCmd.device = std::stoul(*p);
                if (auto p = getParam(f, Keywords::Params::Id))     parseId(*p, ignoreLvl, sensCmd.id_val);

                // 2. Зчитуємо period та toEmit
                if (auto p = getParam(f, Keywords::Params::Period)) sensCmd.period = std::stoull(*p);
                if (auto p = getParam(f, Keywords::Params::ToEmit)) sensCmd.valueToEmit = std::stod(*p);

                generated.emplace_back(sensCmd);
            }
            else if (cmd == Keywords::Read) {
                std::string val = trim(rawCmd.substr(Keywords::Read.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                if (val.front() == '"' && val.back() == '"') val = val.substr(1, val.length() - 2);

                ConfigCmdRead readCmd{};
                std::strncpy(readCmd.filename, val.c_str(), sizeof(readCmd.filename) - 1);
                readCmd.filename[sizeof(readCmd.filename) - 1] = '\0';
                generated.emplace_back(readCmd);
            }
            else if (cmd == Keywords::Node) {
                std::string val = trim(rawCmd.substr(Keywords::Node.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                auto f = parseStructContent(val, env, {Keywords::Params::Device, Keywords::Params::Id, Keywords::Params::Algo, Keywords::Params::Output, Keywords::Params::Inputs});

                ConfigCmdNodeCreate createNode{};
                createNode.device = currentDevice;

                if (auto p = getParam(f, Keywords::Params::Device)) createNode.device = std::stoul(*p);
                if (auto p = getParam(f, Keywords::Params::Id))     parseId(*p, createNode.id_lvl, createNode.id_val);

                createNode.algo = 0; // Значення за замовчуванням
                if (auto p = getParam(f, Keywords::Params::Algo)) {
                    std::string_view algoName = *p;
                    if (algoName.size() >= 2 && algoName.front() == '"' && algoName.back() == '"') {
                        algoName = algoName.substr(1, algoName.length() - 2);
                    }
                    createNode.algo = AlgoNames::algoId(algoName);
                }

                if (auto outStr = getParam(f, Keywords::Params::Output)) {
                    auto outF = parseStructContent(*outStr, env, {Keywords::Params::OutC, Keywords::Params::OutA});
                    if (auto p = getParam(outF, Keywords::Params::OutC)) createNode.output.c = parseC(*p);
                    if (auto p = getParam(outF, Keywords::Params::OutA)) createNode.output.a = std::stod(*p);
                }

                std::vector<std::string> inputs;
                if (auto inStr = getParam(f, Keywords::Params::Inputs)) {
                    std::string arrStr = *inStr;
                    if (arrStr.front() == '[') arrStr = arrStr.substr(1, arrStr.length() - 2);
                    inputs = splitTokens(arrStr);
                }

                createNode.inputs_count = inputs.size();
                generated.emplace_back(createNode);

                for (std::size_t i = 0; i < inputs.size(); ++i) {
                    auto it = env.find(inputs[i]);
                    std::string inVal = (it != env.end()) ? it->second : inputs[i];
                    auto inF = parseStructContent(inVal, env, {Keywords::Params::InC, Keywords::Params::InA, Keywords::Params::InB, Keywords::Params::InV, Keywords::Params::InG});

                    ConfigCmdNodeAddInput addIn{};
                    addIn.device = createNode.device;
                    addIn.id_lvl = createNode.id_lvl;
                    addIn.id_val = createNode.id_val;
                    addIn.input_index = i;

                    if (auto p = getParam(inF, Keywords::Params::InC)) addIn.input_data.c_i = parseC(*p);
                    if (auto p = getParam(inF, Keywords::Params::InA)) addIn.input_data.a_i = std::stod(*p);
                    if (auto p = getParam(inF, Keywords::Params::InB)) addIn.input_data.b_i = std::stoul(*p);
                    if (auto p = getParam(inF, Keywords::Params::InV)) addIn.input_data.v_i = std::stod(*p);
                    if (auto p = getParam(inF, Keywords::Params::InG)) addIn.input_data.g_i = std::stod(*p);

                    generated.emplace_back(addIn);
                }
            }
            else if (cmd == Keywords::BindE) {
                // Новий синтаксис: bindE <deviceId=... id=... to=...>
                std::string val = trim(rawCmd.substr(Keywords::BindE.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                auto f = parseStructContent(val, env, {Keywords::Params::Device, Keywords::Params::Id, Keywords::Params::To});

                ConfigCmdBindSingle single{};
                single.from.device = currentDevice;
                if (auto p = getParam(f, Keywords::Params::Device)) single.from.device = std::stoul(*p);
                if (auto p = getParam(f, Keywords::Params::Id))     parseId(*p, single.from.id_lvl, single.from.id_val);
                if (auto p = getParam(f, Keywords::Params::To))     single.deviceTo = std::stoul(*p);

                generated.emplace_back(single);
            }
            else if (cmd == Keywords::Bind) {
                // Новий синтаксис: bind <deviceId=... id=... to=[...]>
                std::string val = trim(rawCmd.substr(Keywords::Bind.length()));
                if (auto it = env.find(val); it != env.end()) val = it->second;
                auto f = parseStructContent(val, env, {Keywords::Params::Device, Keywords::Params::Id, Keywords::Params::To});

                KgBindFrom src{};
                src.device = currentDevice;
                if (auto p = getParam(f, Keywords::Params::Device)) src.device = std::stoul(*p);
                if (auto p = getParam(f, Keywords::Params::Id))     parseId(*p, src.id_lvl, src.id_val);

                if (auto toStrPtr = getParam(f, Keywords::Params::To)) {
                    std::string toStr = *toStrPtr;
                    if (!toStr.empty() && toStr.front() == '[') toStr = toStr.substr(1, toStr.length() - 2);
                    auto toArr = splitTokens(toStr);

                    for (const auto& toValRaw : toArr) {
                        std::string toVal = toValRaw;
                        if (auto it = env.find(toVal); it != env.end()) toVal = it->second;

                        auto fTo = parseStructContent(toVal, env, {Keywords::Params::Device, Keywords::Params::Id, Keywords::Params::In});

                        ConfigCmdBindDouble link{};
                        link.from = src;
                        link.to.device = currentDevice;
                        if (auto p = getParam(fTo, Keywords::Params::Device)) link.to.device = std::stoul(*p);
                        if (auto p = getParam(fTo, Keywords::Params::Id))     parseId(*p, link.to.id_lvl, link.to.id_val);
                        if (auto p = getParam(fTo, Keywords::Params::In))     link.to.in_index = std::stoul(*p);

                        generated.emplace_back(link);
                    }
                }
            }

            return generated;
        }
    };
}
