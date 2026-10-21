#pragma once
#include <tuple>

namespace NodeSystem::strings::convert
{
    template <typename... Fields>
    struct Scheme
    {
    private:
        template <typename PtrType>
        struct ClassExtractor;

        template <typename Class, typename T>
        struct ClassExtractor<T Class::*>
        {
            using type = Class;
        };

        using first_field_t = std::tuple_element_t<0, std::tuple<Fields...>>;

    public:
        using target_t = ClassExtractor<typename first_field_t::pointer_t>::type;
        std::tuple<Fields...> fields;

        constexpr Scheme(Fields... f) : fields(f...)
        {
        }
    };
}
