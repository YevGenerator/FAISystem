#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <any>
#include <optional>
#include <iostream>

namespace NodeSystem::strings {
    class ParserContext {
        std::unordered_map<std::string, std::any> constants;
    public:
        template<typename T>
        void setConstant(std::string_view name, T value) {
            constants.emplace(std::string(name), std::move(value));
        }

        template<typename T>
        std::optional<T> resolve(std::string_view name) const {
            auto it = constants.find(std::string(name));
            if (it != constants.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (const std::bad_any_cast&) {
                    std::cerr << "Type mismatch for constant: " << name << '\n';
                }
            }
            return std::nullopt;
        }
    };
}