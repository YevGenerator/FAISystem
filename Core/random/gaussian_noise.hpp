#pragma once

#include <optional>
#include <random>

#include "types/basic.hpp"

namespace NodeSystem::Core::Random {
    using EngineType = std::conditional_t<
        sizeof(types::SeedInt) == sizeof(std::uint64_t),
        std::mt19937_64,
        std::mt19937
    >;

    class GaussianNoise {
        EngineType random_engine;
        types::SeedInt _seed = 0;

    public:
        auto operator()(const double mean, const double std_dev) -> double {
            std::normal_distribution dist(mean, std_dev);
            return dist(random_engine);
        }

        void setSeed(const std::optional<types::SeedInt> newSeed = std::nullopt,
                     const std::optional<types::SeedInt> additionalInfo = std::nullopt) {
            if (newSeed.has_value()) {
                if (additionalInfo.has_value()) {
                    this->_seed = *newSeed ^ std::hash<types::SeedInt>{}(*additionalInfo);
                } else {
                    this->_seed = *newSeed;
                }
            } else {
                this->_seed = std::random_device{}();
            }
            this->random_engine.seed(this->_seed);
        }

        [[nodiscard]]
        auto seed() const -> types::SeedInt {
            return this->_seed;
        }
    };

} // namespace NodeSystem::Core::Random
