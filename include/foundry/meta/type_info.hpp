#ifndef FOUNDRY_META_TYPE_INFO_H_
#define FOUNDRY_META_TYPE_INFO_H_

#include <cstddef>
#include <cstdint>

#include <string_view>

#include <type_traits>
#include <concepts>

#include <source_location>

#include <foundry/meta/underlying_type.hpp>

#include <foundry/compile_time.hpp>

namespace foundry::meta {
    template<typename T, auto U>
        requires(std::derived_from<decltype(U), detail::underlying_type_base>)
    inline constexpr bool instance_of() noexcept {
        return std::same_as<decltype(extract_underlying_type<T>()), decltype(U)>;
    }
}

#include <foundry/meta/template_info.hpp>
#include <foundry/template_lists.hpp>

namespace foundry::meta {
    template<typename T, auto U, typename... Args>
        requires(sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return get_template_info<T>().templates.starts_with<make_mixed_template<Args>()...>();
    }

    template<typename T, auto U, auto... Args>
        requires(sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return get_template_info<T>().templates.starts_with<make_mixed_template<Args>()...>();
    }

    template<typename T, auto U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return get_template_info<T>().templates.starts_with<make_mixed_template<One>(), make_mixed_template<Two>()>();
    }

    template<typename T, auto U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return get_template_info<T>().templates.starts_with<make_mixed_template<One>(), make_mixed_template<Two>()>();
    }
}

#include <foundry/hashing.hpp>
#include <foundry/integral.hpp>

namespace foundry::meta {
    namespace detail {
        template<typename T>
        inline constexpr std::string_view get_concealed_type_name() noexcept {
            return std::source_location::current().function_name();
        }
    }

    class TypeIdentifier {
    private:
        uint128_t _identifier;

        constexpr TypeIdentifier(uint128_t identifier) noexcept : _identifier(identifier) {}

        template<typename T>
        class test1 {};

        template<auto A>
        class test2 {};
    public:
        template<typename T>
        static constexpr TypeIdentifier of() noexcept { 
            static_assert(detail::get_concealed_type_name<double>() == detail::get_concealed_type_name<double>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::get_concealed_type_name<int>() != detail::get_concealed_type_name<float>(), "Type identifiers are not supported on your compiler!");

            static_assert(detail::get_concealed_type_name<test1<double>>() == detail::get_concealed_type_name<test1<double>>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::get_concealed_type_name<test1<int>>() != detail::get_concealed_type_name<test1<float>>(), "Type identifiers are not supported on your compiler!");

            static_assert(detail::get_concealed_type_name<test2<1.0f>>() == detail::get_concealed_type_name<test2<1.0f>>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::get_concealed_type_name<test2<1>>() != detail::get_concealed_type_name<test2<1.0f>>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::get_concealed_type_name<test2<1u>>() != detail::get_concealed_type_name<test2<2u>>(), "Type identifiers are not supported on your compiler!");

            return TypeIdentifier(murmur128::hash(detail::get_concealed_type_name<std::remove_cvref_t<T>>()));
        }

        constexpr TypeIdentifier& operator=(const TypeIdentifier& rhs) noexcept {
            _identifier = rhs._identifier;
            return *this;
        }

        constexpr bool operator==(const TypeIdentifier& rhs) const noexcept {
            return _identifier == rhs._identifier;
        }

        constexpr std::strong_ordering operator<=>(const TypeIdentifier& rhs) const noexcept = delete;

        constexpr uint128_t as_int() const noexcept {
            return _identifier;
        }
    };

    template<typename T>
    inline constexpr TypeIdentifier type_id() noexcept {
        return TypeIdentifier::of<T>();
    }

    template<typename T>
    concept is_structural = requires {
        [] <T>() {};
    };
}

#endif