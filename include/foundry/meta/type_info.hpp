#ifndef FOUNDRY_TYPE_INFO_H_
#define FOUNDRY_TYPE_INFO_H_

#include <cstddef>
#include <cstdint>

#include <string_view>

#include <type_traits>
#include <concepts>

#include <source_location>

#include <foundry/compile_time.hpp>

namespace Foundry::Meta {
    namespace detail {
        template<template<typename...> class T, typename U>
        struct instance_of_helper : std::false_type {};

        template<template<typename...> class T, typename... Args>
        struct instance_of_helper<T, T<Args...>> : std::true_type {};

        template<template<auto...> class T, typename U>
        struct instance_of_helper_2 : std::false_type {};

        template<template<auto...> class T, auto... Args>
        struct instance_of_helper_2<T, T<Args...>> : std::true_type {};

        template<template<typename, auto> class T, typename U>
        struct instance_of_helper_3 : std::false_type {};

        template<template<typename, auto> class T, typename One, auto Two>
        struct instance_of_helper_3<T, T<One, Two>> : std::true_type {};

        template<template<auto, typename> class T, typename U>
        struct instance_of_helper_4 : std::false_type {};

        template<template<auto, typename> class T, auto One, typename Two>
        struct instance_of_helper_4<T, T<One, Two>> : std::true_type {};
    }

    template<typename T, template<typename...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper<U, std::remove_cvref_t<T>>::value;
    }

    template<typename T, template<auto...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_2<U, std::remove_cvref_t<T>>::value;
    }

    template<typename T, template<typename, auto> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_3<U, std::remove_cvref_t<T>>::value;
    }

    template<typename T, template<auto, typename> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_4<U, std::remove_cvref_t<T>>::value;
    }

    template<auto Value, template<typename...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper<U, std::remove_cvref_t<decltype(Value)>>::value;
    }

    template<auto Value, template<auto...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_2<U, std::remove_cvref_t<decltype(Value)>>::value;
    }

    template<auto Value, template<typename, auto> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_3<U, std::remove_cvref_t<decltype(Value)>>::value;
    }

    template<auto Value, template<auto, typename> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_4<U, std::remove_cvref_t<decltype(Value)>>::value;
    }

    template<typename Derived, template<typename...> class Base, typename... Args>
    concept derived_from_instance = requires(Derived * p) {
        [] <typename... Args>(Base<Args...>*) {}(p);
    };
}

#include <foundry/template_lists.hpp>

namespace Foundry::Meta {
    namespace detail {
        template<template<typename...> class T, typename U, typename... Args>
        struct specialized_instance_of_helper : std::false_type {};

        template<template<typename...> class T, typename... Us, typename... Args>
        struct specialized_instance_of_helper<T, T<Us...>, Args...> : std::bool_constant<TypeList<Us...>::template startsWith<Args...>()> {};

        template<template<auto...> class T, typename U, auto... Args>
        struct specialized_instance_of_helper_2 : std::false_type {};

        template<template<auto...> class T, auto... Us, auto... Args>
        struct specialized_instance_of_helper_2<T, T<Us...>, Args...> : std::bool_constant<ValueList<Us...>::template startsWith<Args...>()> {};

        template<template<typename, auto> class T, typename U, typename One, auto Two>
        struct specialized_instance_of_helper_3 : std::false_type {};

        template<template<typename, auto> class T, typename One, auto Two>
        struct specialized_instance_of_helper_3<T, T<One, Two>, One, Two> : std::true_type {};

        template<template<auto, typename> class T, typename U, auto One, typename Two>
        struct specialized_instance_of_helper_4 : std::false_type {};

        template<template<auto, typename> class T, auto One, typename Two>
        struct specialized_instance_of_helper_4<T, T<One, Two>, One, Two> : std::true_type {};
    }

    template<typename T, template<typename...> class U, typename... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper<U, std::remove_cvref_t<T>, Args...>::value;
    }

    template<typename T, template<auto...> class U, auto... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_2<U, std::remove_cvref_t<T>, Args...>::value;
    }

    template<typename T, template<typename, auto> class U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_3<U, std::remove_cvref_t<T>, One, Two>::value;
    }

    template<typename T, template<auto, typename> class U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_4<U, std::remove_cvref_t<T>, One, Two>::value;
    }

    template<auto Value, template<typename...> class U, typename... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper<U, std::remove_cvref_t<decltype(Value)>, Args...>::value;
    }

    template<auto Value, template<auto...> class U, auto... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_2<U, std::remove_cvref_t<decltype(Value)>, Args...>::value;
    }

    template<auto Value, template<typename, auto> class U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_3<U, std::remove_cvref_t<decltype(Value)>, One, Two>::value;
    }

    template<auto Value, template<auto, typename> class U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_4<U, std::remove_cvref_t<decltype(Value)>, One, Two>::value;
    }
}

#include <foundry/integral.hpp>

namespace Foundry::Meta {
    namespace detail {
        template<typename T>
        inline constexpr std::string_view getConcealedTypeName() noexcept {
            return std::source_location::current().function_name();
        }
    }

    class TypeIdentifier {
    private:
        uint128_t _identifier;

        inline constexpr TypeIdentifier(uint128_t identifier) noexcept : _identifier(identifier) {}
    public:
        template<typename T>
        inline static constexpr TypeIdentifier of() noexcept {
            static_assert(detail::getConcealedTypeName<double>() == detail::getConcealedTypeName<double>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::getConcealedTypeName<int>() != detail::getConcealedTypeName<float>(), "Type identifiers are not supported on your compiler!");

            return TypeIdentifier(Murmur128::hash(detail::getConcealedTypeName<std::remove_cvref_t<T>>()));
        }

        inline constexpr TypeIdentifier& operator=(const TypeIdentifier& rhs) noexcept {
            _identifier = rhs._identifier;
            return *this;
        }

        inline constexpr bool operator==(const TypeIdentifier& rhs) const noexcept {
            return _identifier == rhs._identifier;
        }

        inline constexpr std::strong_ordering operator<=>(const TypeIdentifier& rhs) const noexcept = delete;
    };

    template<typename T>
    inline constexpr TypeIdentifier typeId() noexcept {
        return TypeIdentifier::of<T>();
    }
}

#endif