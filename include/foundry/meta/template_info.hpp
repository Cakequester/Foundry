#ifndef FOUNDRY_TEMPLATE_INFO_H_
#define FOUNDRY_TEMPLATE_INFO_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <foundry/meta/type_info.hpp>

#include <foundry/compile_time.hpp>
#include <foundry/template_lists.hpp>

namespace Foundry::Meta {
	template<typename Ts, typename Vs>
		requires(instance_of<Ts, TypeList>() && instance_of<Vs, ValueList>())
	class TemplateInfo {
	public:
		inline static constexpr Ts Types;
		inline static constexpr Vs Values;
	};
	
	namespace detail {
		template<typename T, std::size_t N>
		struct template_arg {
			static_assert(Foundry::template_failure<T>(), "T is too irregular for template info!");
		};

		template<template<typename...> class U, typename... Args, std::size_t N>
		struct template_arg<U<Args...>, N> {
			inline static constexpr auto Info = getPackItemAt<N, Args...>();
		};

		template<template<auto...> class U, auto... Args, std::size_t N>
		struct template_arg<U<Args...>, N> {
			inline static constexpr auto Info = getPackItemAt<N, Args...>();
		};

		template<template<typename, auto> class U, typename One, auto Two, std::size_t N>
		struct template_arg<U<One, Two>, N> {
			inline static constexpr auto Info = getPackItemAt<N, One, Two>();
		};

		template<template<auto, typename> class U, auto One, typename Two, std::size_t N>
		struct template_arg<U<One, Two>, N> {
			inline static constexpr auto Info = getPackItemAt<N, One, Two>();
		};

		template<typename T>
		struct template_count {
			static_assert(Foundry::template_failure<T>(), "T is too irregular for template info!");
		};

		template<template<typename...> class U, typename... Args>
		struct template_count<U<Args...>> : std::integral_constant<std::size_t, sizeof...(Args)> {};

		template<template<auto...> class U, auto... Args>
		struct template_count<U<Args...>> : std::integral_constant<std::size_t, sizeof...(Args)> {};

		template<template<typename, auto> class U, typename One, auto Two>
		struct template_count<U<One, Two>> : std::integral_constant<std::size_t, 2> {};

		template<template<auto, typename> class U, auto One, typename Two>
		struct template_count<U<One, Two>> : std::integral_constant<std::size_t, 2> {};

		template<typename T, std::size_t Count, std::size_t N, typename List>
			requires(instance_of<List, TypeList>())
		inline constexpr auto getTemplateTypesHelper() noexcept {
			if constexpr (N == Count) {
				return List{};
			} else {
				constexpr auto Info = template_arg<T, N>::Info;
				if constexpr (Info.IsType) {
					using Next = decltype(List::template append<typename decltype(Info.Type.value)::type>());
					return getTemplateTypesHelper<T, Count, N + 1, Next>();
				}
				else return getTemplateTypesHelper<T, Count, N + 1, List>();
			}
		}

		template<typename T>
		inline constexpr auto getTemplateTypes() noexcept {
			return getTemplateTypesHelper<T, detail::template_count<T>::value, 0, TypeList<>>();
		}

		template<typename T, std::size_t Count, std::size_t N, typename List>
			requires(instance_of<List, ValueList>())
		inline constexpr auto getTemplateValuesHelper() noexcept {
			if constexpr (N == Count) {
				return List{};
			} else {
				constexpr auto Info = template_arg<T, N>::Info;
				if constexpr (Info.IsValue) {
					using Next = decltype(List::template append<Info.Value.value>());
					return getTemplateValuesHelper<T, Count, N + 1, Next>();
				}
				else return getTemplateValuesHelper<T, Count, N + 1, List>();
			}
		}

		template<typename T>
		inline constexpr auto getTemplateValues() noexcept {
			return getTemplateValuesHelper<T, detail::template_count<T>::value, 0, ValueList<>>();
		}
	}

	template<typename T>
	inline constexpr auto getTemplateInfo() noexcept {
		using Types = std::remove_cvref_t<decltype(detail::getTemplateTypes<T>())>;
		using Values = std::remove_cvref_t<decltype(detail::getTemplateValues<T>())>;

		return TemplateInfo<Types, Values>();
	}
}

#endif