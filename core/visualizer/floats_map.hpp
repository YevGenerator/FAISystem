#pragma once
#include <string_view>
#include <unordered_map>

#include "float_names.hpp"

namespace NodeSystem::Visual {
    class FloatsMap {
        std::unordered_map<std::string_view, float> floats{};

    public:
        float &operator[](const std::string_view name) {
            return this->floats[name];
        }

        void init(const std::string_view key, const float value) {
            this->floats[key] = value;
        }

        auto& map() {
            return this->floats;
        }

        void init() {
            namespace f = Keys::Floats;
            this->init(f::zoom, 1.0f);
            this->init(f::width, 200.0f);
            this->init(f::minBodyHeight, 50.0f);
            this->init(f::titleHeight, 25.0f);
            this->init(f::inputSlotHeight, 25.0f);
            this->init(f::connectorSize, (*this)[f::inputSlotHeight] / 3.0f);
            this->init(f::fontSize, 16.0f);
            this->init(f::slotFontSize, 14.0f);
            this->init(f::borderThick, 3.0f);
            this->init(f::borderThin, 1.0f);
            this->init(f::textPaddingX, 10.0f);
            this->init(f::textPaddingY, 5.0f);
            this->init(f::columnSpacing, (*this)[f::width] * 1.0f);
            this->init(f::nodeSpacing, 40.0f);
            this->init(f::devicePadding, 10.0f);
            this->init(f::linkThick, 3.0f);
            this->init(f::linkThin, 1.5f);
        }
    };
    static FloatsMap floatsMap{};
}
