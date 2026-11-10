#ifndef FOUNDRY_UNDERLYING_TYPE_H_
#define FOUNDRY_UNDERLYING_TYPE_H_

#include <type_traits>
#include <concepts>

namespace Foundry::Meta {
    namespace detail {
        class UnderlyingTypeBase {};

        template<template<typename...> class T>
        class TypenamesUnderlyingType : public UnderlyingTypeBase {
        public:
            template<typename... Args> using apply = T<Args...>;
        };

        template<template<auto...> class T>
        class ValuesUnderlyingType : public UnderlyingTypeBase {
        public:
            template<auto... Args> using apply = T<Args...>;
        };

        template<template<typename, auto> class T>
        class TypenameValueUnderlyingType : public UnderlyingTypeBase {
        public:
            template<typename One, auto Two>
            using apply = T<One, Two>;
        };

        template<template<auto, typename> class T>
        class ValueTypenameUnderlyingType : public UnderlyingTypeBase {
        public:
            template<auto One, typename Two>
            using apply = T<One, Two>;
        };

        template<typename T>
        class extract_underlying_type {
        public:
            using type = T;
        };

        template<template<typename...> class T, typename... Args>
        class extract_underlying_type<T<Args...>> {
        public:
            using type = TypenamesUnderlyingType<T>;
        };

        template<template<auto...> class T, auto... Args>
        class extract_underlying_type<T<Args...>> {
        public:
            using type = ValuesUnderlyingType<T>;
        };

        template<template<typename, auto> class T, typename One, auto Two>
        class extract_underlying_type<T<One, Two>> {
        public:
            using type = TypenameValueUnderlyingType<T>;
        };

        template<template<auto, typename> class T, auto One, typename Two>
        class extract_underlying_type<T<One, Two>> {
        public:
            using type = ValueTypenameUnderlyingType<T>;
        };
    }

    template<template<typename...> class T>
    inline constexpr detail::TypenamesUnderlyingType<T> toUnderlyingType() noexcept {
        return {};
    }

    template<template<auto...> class T>
    inline constexpr detail::ValuesUnderlyingType<T> toUnderlyingType() noexcept {
        return {};
    }

    template<template<typename, auto> class T>
    inline constexpr detail::TypenameValueUnderlyingType<T> toUnderlyingType() noexcept {
        return {};
    }

    template<template<auto, typename> class T>
    inline constexpr detail::ValueTypenameUnderlyingType<T> toUnderlyingType() noexcept {
        return {};
    }

    template<typename T>
    inline constexpr typename detail::extract_underlying_type<T>::type extractUnderlyingType() noexcept {
        return {};
    }
}

#endif