#ifndef FOUNDRY_TYPE_INFO_H_
#define FOUNDRY_TYPE_INFO_H_

#include <cstddef>
#include <cstdint>

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
        return detail::instance_of_helper<U, T>::value;
    }

    template<typename T, template<auto...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_2<U, T>::value;
    }

    template<typename T, template<typename, auto> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_3<U, T>::value;
    }

    template<typename T, template<auto, typename> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_4<U, T>::value;
    }

    template<auto Value, template<typename...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper<U, decltype(Value)>::value;
    }

    template<auto Value, template<auto...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_2<U, decltype(Value)>::value;
    }

    template<auto Value, template<typename, auto> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_3<U, decltype(Value)>::value;
    }

    template<auto Value, template<auto, typename> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::instance_of_helper_4<U, decltype(Value)>::value;
    }

    template<typename Derived, template<typename...> class Base, typename... Args>
    concept derived_from_instance = requires(Derived * p) {
        [] <typename... Args>(Base<Args...>*) {}(p);
    };

    namespace detail {
        template<typename U>
        static consteval const char* typeIdRawName() noexcept {
            return std::source_location::current().function_name();
        }
    }

    template<typename U>
    static consteval auto typeId() noexcept {
        constexpr const char* name = detail::typeIdRawName<U>();
        return makeFixedStringFromPointer<std::char_traits<char>::length(name) + 1>(name);
    }
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
        return detail::specialized_instance_of_helper<U, T, Args...>::value;
    }

    template<typename T, template<auto...> class U, auto... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_2<U, T, Args...>::value;
    }

    template<typename T, template<typename, auto> class U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_3<U, T, One, Two>::value;
    }

    template<typename T, template<auto, typename> class U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_4<U, T, One, Two>::value;
    }

    template<auto Value, template<typename...> class U, typename... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper<U, decltype(Value), Args...>::value;
    }

    template<auto Value, template<auto...> class U, auto... Args>
        requires (sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_2<U, decltype(Value), Args...>::value;
    }

    template<auto Value, template<typename, auto> class U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_3<U, decltype(Value), One, Two>::value;
    }

    template<auto Value, template<auto, typename> class U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        return detail::specialized_instance_of_helper_4<U, decltype(Value), One, Two>::value;
    }
}

#endif