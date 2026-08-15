#pragma once
#include "Sensor.hpp"
#include <ranges>
#include <vector>

#include "PinDriver.hpp"

namespace NodeSystem::Core::Sensors {
    template<typename TFunc, typename TArg>
    concept IsInvokable = std::invocable<TFunc, const TArg &>;

    struct SensorStore {
    private:
        std::vector<Sensor> sensorCollection;

    public:
        auto sensors() -> auto {
            return std::views::all(sensorCollection);
        }

        auto add(const types::ID sensorId, const types::ID pinId) {
            this->sensorCollection.emplace_back(sensorId, pinId);
        }

        auto emitSensors(IsInvokable<const Nodes::NodeResultMessage &> auto &&callback) {
            for (auto &sensor: sensors()) {
                const auto value = PinDriver::readPin(sensor.pinIndex);
                auto message = sensor.asMessage(value);
                if (message.has_value()) {
                    callback(message.value());
                }
            }
        }
    };
} // namespace NodeSystem::Core::Sensors
