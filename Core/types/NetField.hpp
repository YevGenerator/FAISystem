#pragma once
#include <bit>
#include <cstdint>

namespace NodeSystem::Core::types {
    template<typename Origin, typename Raw>
    struct NetField {
        static_assert(sizeof(Origin) == sizeof(Raw));
        static_assert(std::is_trivially_copyable_v<Origin>);
        static_assert(std::is_trivially_copyable_v<Raw>);

        Raw raw{};

        constexpr NetField() noexcept = default;

        constexpr NetField(Origin value) noexcept
            : raw(std::bit_cast<Raw>(value)) {
        }

        [[nodiscard]]
        constexpr auto get() const noexcept -> Origin {
            return std::bit_cast<Origin>(this->raw);
        }

        constexpr void set(Origin value) noexcept {
            this->raw = std::bit_cast<Raw>(value);
        }

        constexpr auto operator=(Origin value) noexcept -> NetField & {
            this->raw = std::bit_cast<Raw>(value);
            return *this;
        }

        constexpr operator Origin() const noexcept {
            return std::bit_cast<Origin>(this->raw);
        }
    };

    using NetDouble = NetField<double, std::uint64_t>;
    using NetBool = NetField<bool, std::uint8_t>;
} // namespace NodeSystem::Core::types
