#pragma once
#include <string_view>

namespace NodeSystem::strings::dto
{
    template <typename T>
    concept IsDTO = requires
    {
        T::value_type;
        { T::name } -> std::same_as<std::string_view>;
    };

    template <typename InnerValue, const std::string_view& Name>
    struct BaseDTO
    {
        using value_type = InnerValue;
        static constexpr std::string_view name = Name;

        InnerValue value{};

        constexpr BaseDTO() = default;

        constexpr BaseDTO(const value_type& val) : value(val)
        {
        }

        constexpr BaseDTO(value_type&& val) noexcept : value(std::move(val))
        {
        }

        constexpr operator value_type&() noexcept { return value; }
        constexpr operator const value_type&() const noexcept { return value; }

        constexpr value_type* operator->() noexcept { return &value; }
        constexpr const value_type* operator->() const noexcept { return &value; }

        constexpr value_type& get() noexcept { return value; }
        constexpr const value_type& get() const noexcept { return value; }

        constexpr auto operator<=>(const BaseDTO&) const = default;
    };
}
