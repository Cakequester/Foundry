#ifndef FOUNDRY_META_UNDERLYING_TYPE_H_
#define FOUNDRY_META_UNDERLYING_TYPE_H_

#include <type_traits>
#include <concepts>

namespace foundry::meta {
    namespace detail {
        class underlying_type_base {};

        template<template<typename...> class T>
        class typenames_underlying_type : public underlying_type_base {
        public:
            template<typename... Args> using apply = T<Args...>;
        };

        template<template<auto...> class T>
        class values_underlying_type : public underlying_type_base {
        public:
            template<auto... Args> using apply = T<Args...>;
        };

        template<template<typename, auto> class T>
        class typename_value_underlying_type : public underlying_type_base {
        public:
            template<typename One, auto Two>
            using apply = T<One, Two>;
        };

        template<template<auto, typename> class T>
        class value_typename_underlying_type : public underlying_type_base {
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
            using type = typenames_underlying_type<T>;
        };

        template<template<auto...> class T, auto... Args>
        class extract_underlying_type<T<Args...>> {
        public:
            using type = values_underlying_type<T>;
        };

        template<template<typename, auto> class T, typename One, auto Two>
        class extract_underlying_type<T<One, Two>> {
        public:
            using type = typename_value_underlying_type<T>;
        };

        template<template<auto, typename> class T, auto One, typename Two>
        class extract_underlying_type<T<One, Two>> {
        public:
            using type = value_typename_underlying_type<T>;
        };
    }

    template<template<typename...> class T>
    inline constexpr detail::typenames_underlying_type<T> to_underlying_type() noexcept {
        return {};
    }

    template<template<auto...> class T>
    inline constexpr detail::values_underlying_type<T> to_underlying_type() noexcept {
        return {};
    }

    template<template<typename, auto> class T>
    inline constexpr detail::typename_value_underlying_type<T> to_underlying_type() noexcept {
        return {};
    }

    template<template<auto, typename> class T>
    inline constexpr detail::value_typename_underlying_type<T> to_underlying_type() noexcept {
        return {};
    }

    template<typename T>
    inline constexpr typename detail::extract_underlying_type<T>::type extract_underlying_type() noexcept {
        return {};
    }
}

#endif