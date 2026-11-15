#ifndef FOUNDRY_META_TEMPLATE_INFO_H_
#define FOUNDRY_META_TEMPLATE_INFO_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <utility>

#include <foundry/meta/underlying_type.hpp>
#include <foundry/meta/type_info.hpp>

#include <foundry/compile_time.hpp>
#include <foundry/template_lists.hpp>

namespace foundry::meta {
    template<typename Ordered>
        requires(instance_of<Ordered, to_underlying_type<mixed_template_list>()>())
    class TemplateInfo {
    public:
        inline static constexpr Ordered templates{};
    };
    
    namespace detail {
        template<typename T, std::size_t N>
        struct template_arg {
            static_assert(false, "T is too irregular for template info!");
        };

        template<template<typename...> class U, typename... Args, std::size_t N>
        struct template_arg<U<Args...>, N> {
            inline static constexpr auto info = mixed_template_list<make_mixed_template<Args>()...>::template at<N>();
        };

        template<template<auto...> class U, auto... Args, std::size_t N>
        struct template_arg<U<Args...>, N> {
            inline static constexpr auto info = mixed_template_list<make_mixed_template<Args>()...>::template at<N>();
        };

        template<template<typename, auto> class U, typename One, auto Two, std::size_t N>
        struct template_arg<U<One, Two>, N> {
            inline static constexpr auto info = mixed_template_list<make_mixed_template<One>(), make_mixed_template<Two>()>::template at<N>();
        };

        template<template<auto, typename> class U, auto One, typename Two, std::size_t N>
        struct template_arg<U<One, Two>, N> {
            inline static constexpr auto info = mixed_template_list<make_mixed_template<One>(), make_mixed_template<Two>()>::template at<N>();
        };

        template<typename T>
        struct template_count {
            static_assert(false, "T is too irregular for template info!");
        };

        template<template<typename...> class U, typename... Args>
        struct template_count<U<Args...>> : std::integral_constant<std::size_t, sizeof...(Args)> {};

        template<template<auto...> class U, auto... Args>
        struct template_count<U<Args...>> : std::integral_constant<std::size_t, sizeof...(Args)> {};

        template<template<typename, auto> class U, typename One, auto Two>
        struct template_count<U<One, Two>> : std::integral_constant<std::size_t, 2> {};

        template<template<auto, typename> class U, auto One, typename Two>
        struct template_count<U<One, Two>> : std::integral_constant<std::size_t, 2> {};

        template<typename T, typename ListBuilder, std::size_t Count, std::size_t N = 0>
            requires(instance_of<ListBuilder, to_underlying_type<mixed_template_list_builder>()>())
        inline constexpr auto get_ordered_list_from_template() noexcept {
            if constexpr (N == Count) return std::type_identity<ListBuilder>{};
            else {
                constexpr auto Info = template_arg<T, N>::info;
                constexpr std::size_t Next = N + 1;
                if constexpr (Info.HasType)
                    return get_ordered_list_from_template<T, decltype(ListBuilder::template add<typename decltype(Info)::type_holder::type>()), Count, Next>();
                else
                    return get_ordered_list_from_template<T, decltype(ListBuilder::template add<Info.value()>()), Count, Next>();
            }
        }
    }

    template<typename T>
    inline constexpr auto get_template_info() noexcept {
        return TemplateInfo<decltype(decltype(detail::get_ordered_list_from_template<T, mixed_template_list_builder<>, detail::template_count<T>::value>())::type::build())>();
    }
}

#endif