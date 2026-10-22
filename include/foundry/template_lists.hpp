#ifndef FOUNDRY_TEMPLATE_LISTS_H_
#define FOUNDRY_TEMPLATE_LISTS_H_

#include <cstddef>
#include <cstdint>

#include <optional>

#include <type_traits>
#include <concepts>

#include <utility>

#include <foundry/compile_time.hpp>
#include <foundry/meta/type_info.hpp>

namespace Foundry {
    namespace detail {
        template<std::size_t Index, typename Current, typename... Rest>
        inline constexpr auto getTypeAtImplementation() noexcept {
            if constexpr (Index == 0) return std::type_identity<Current>{};
            else return getTypeAtImplementation<Index - 1, Rest...>();
        }
    }

    template<std::size_t Index, typename... Ts>
    inline constexpr auto getTypeAt() noexcept {
        static_assert(Index < sizeof...(Ts), "Index is out of bounds.");
        return detail::getTypeAtImplementation<Index, Ts...>();
    }

    template<typename T, typename... Ts>
    inline constexpr bool containsType() noexcept {
        return (std::same_as<T, Ts> || ...);
    }

    template<typename T, typename... Ts>
    inline constexpr std::optional<std::size_t> indexOfType() noexcept {
        std::size_t i = 0;
        std::optional<std::size_t> result;
        (((std::same_as<T, Ts> && !result) ? (result = i, true) : (++i, false)) || ...);
        return result;
    }

    template<typename... Ts>
    class TypeList {
    private:
        // Could be lambdas, but MSVC is bad about immediately invoked lambdas.
        template<std::size_t... Is, typename... Us>
        inline static constexpr bool startsWithImpl(std::index_sequence<Is...>, TypeList<Us...>*) noexcept {
            return (std::is_same_v<at<Is>, Us> && ...);
        }

        template<typename... Us>
        inline static constexpr bool startsWithListImpl(TypeList<Us...>*) noexcept {
            return startsWith<Us...>();
        }

        template<std::size_t... I>
        inline static constexpr auto reverseImpl(std::index_sequence<I...>) noexcept {
            return TypeList<at<sizeof...(Ts) - 1 - I>...>{};
        }

        template<std::size_t Start, std::size_t... I>
        inline static constexpr auto sublistImpl(std::index_sequence<I...>) noexcept {
            return TypeList<at<Start + I>...>{};
        }

        template<typename... Others>
        inline static constexpr auto appendListImpl(TypeList<Others...>*) noexcept {
            return TypeList<Ts..., Others...>{};
        }

        template<std::size_t Skip, std::size_t... I>
        inline static constexpr auto removeAtImpl(std::index_sequence<I...>) noexcept {
            return TypeList<at<(I < Skip ? I : I + 1)>...>{};
        }
    public:
        template<typename...>
        friend class TypeList;

        template<std::size_t Index>
        using at = typename decltype(getTypeAt<Index, Ts...>())::type;

        inline static constexpr auto front() noexcept {
            return std::type_identity<at<0>>{};
        }

        inline static constexpr auto back() noexcept {
            return std::type_identity<at<size() - 1>>{};
        }

        template<typename T>
        inline static constexpr bool contains() noexcept {
            return containsType<T, Ts...>();
        }

        template<typename... Us>
        inline static constexpr bool startsWith() noexcept {
            if constexpr (sizeof...(Us) > sizeof...(Ts)) return false;
            else return startsWithImpl(std::index_sequence_for<Us...>{}, static_cast<TypeList<Us...>*>(nullptr));
        }

        template<typename Other>
            requires(Meta::instance_of<Other, TypeList>())
        static constexpr bool startsWithList() noexcept {
            return startsWithListImpl(static_cast<Other*>(nullptr));
        }

        template<typename T>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            return indexOfType<T, Ts...>();
        }

        inline static constexpr auto reverse() noexcept {
            return reverseImpl(std::make_index_sequence<sizeof...(Ts)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Ts), "Invalid range.");
            return sublistImpl<Start>(std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<typename T>
        inline static constexpr auto pushFront() noexcept {
            return TypeList<T, Ts...>{};
        }

        template<typename... Us>
        inline static constexpr auto append() noexcept {
            return TypeList<Ts..., Us...>{};
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Foundry::TypeList>())
        inline static constexpr auto appendList() noexcept {
            return appendListImpl(static_cast<Other*>(nullptr));
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<T>();

            if constexpr (!index_of) return TypeList<Ts...>{};
            else return removeAtImpl<*index_of>(std::make_index_sequence<sizeof...(Ts) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Ts);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    namespace detail {
        template<std::size_t Index, auto Current, auto... Rest>
        inline constexpr decltype(auto) getValueAtImplementation() noexcept {
            if constexpr (Index == 0) return Current;
            else return getValueAtImplementation<Index - 1, Rest...>();
        }
    }

    template<std::size_t Index, auto... Values>
    inline constexpr decltype(auto) getValueAt() noexcept {
        static_assert(Index < sizeof...(Values), "Index is out of bounds.");
        return detail::getValueAtImplementation<Index, Values...>();
    }

    template<auto Value, auto... Values>
    inline constexpr bool containsValue() noexcept {
        return ((are_values_equal<Value, Values>()) || ...);
    }

    template<auto Value, auto... Values>
    inline constexpr std::optional<std::size_t> indexOfValue() noexcept {
        std::size_t i = 0;
        std::optional<std::size_t> result;
        (((are_values_equal<Value, Values>() && !result) ? (result = i, true) : (++i, false)) || ...);
        return result;
    }

    template<auto... Values>
    class ValueList {
    private:
        // Could be lambdas, but MSVC is bad about immediately invoked lambdas.
        template<std::size_t... Is, auto... Others>
        inline static constexpr bool startsWithImpl(std::index_sequence<Is...>, ValueList<Others...>*) noexcept {
            return (are_values_equal<at<Is>(), Others>() && ...);
        }

        template<auto... Us>
        inline static constexpr bool startsWithListImpl(ValueList<Us...>*) noexcept {
            return startsWith<Us...>();
        }

        template<std::size_t... I>
        inline static constexpr auto reverseImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<sizeof...(Values) - 1 - I>()...>{};
        }

        template<std::size_t Start, std::size_t... I>
        inline static constexpr auto sublistImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<Start + I>()...>{};
        }

        template<auto... Others>
        inline static constexpr auto appendListImpl(ValueList<Others...>*) noexcept {
            return ValueList<Values..., Others...>{};
        }

        template<std::size_t Skip, std::size_t... I>
        inline static constexpr auto removeAtImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<(I < Skip ? I : I + 1)>()...>{};
        }
    public:
        template<auto...>
        friend class ValueList;

        template<std::size_t Index>
        inline static constexpr decltype(auto) at() noexcept {
            static_assert(Index < sizeof...(Values), "Index out of bounds");
            return getValueAt<Index, Values...>();
        }

        inline static constexpr decltype(auto) front() noexcept {
            return at<0>();
        }

        inline static constexpr decltype(auto) back() noexcept {
            return at<size() - 1>();
        }

        template<auto Value>
        inline static constexpr bool contains() noexcept {
            return containsValue<Value, Values...>();
        }

        template<auto... Others>
        inline static constexpr bool startsWith() noexcept {
            if constexpr (sizeof...(Others) > sizeof...(Values)) return false;
            else return startsWithImpl(std::make_index_sequence<sizeof...(Others)>{}, static_cast<ValueList<Others...>*>(nullptr));
        }

        template<typename Other>
            requires(Meta::instance_of<Other, ValueList>())
        static constexpr bool startsWithList() noexcept {
            return startsWithListImpl(static_cast<Other*>(nullptr));
        }

        template<auto Value>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            return indexOfValue<Value, Values...>();
        }

        inline static constexpr auto reverse() noexcept {
            return reverseImpl(std::make_index_sequence<sizeof...(Values)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Values), "Invalid range.");
            return sublistImpl<Start>(std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<auto Value>
        inline static constexpr auto pushFront() noexcept {
            return ValueList<Value, Values...>{};
        }

        template<auto... Others>
        inline static constexpr auto append() noexcept {
            return ValueList<Values..., Others...>{};
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Foundry::ValueList>())
        inline static constexpr auto appendList() noexcept {
            return appendListImpl(static_cast<Other*>(nullptr));
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<Value>();

            if constexpr (!index_of) return ValueList<Values...>{};
            else return removeAtImpl<*index_of>(std::make_index_sequence<sizeof...(Values) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Values);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    namespace detail {
        struct ParameterPackInfoNull {};
    }

    template<typename T = detail::ParameterPackInfoNull, auto A = detail::ParameterPackInfoNull{} >
    struct ParameterPackInfo {
        inline static constexpr bool IsType = !std::same_as<T, detail::ParameterPackInfoNull>;
        inline static constexpr bool IsValue = !std::same_as<decltype(A), detail::ParameterPackInfoNull>;

        static_assert(IsType || IsValue, "Either a type or value must be supplied!");
        static_assert(!(IsType && IsValue), "A type and value cannot both be supplied!");

        inline static constexpr OptionalField<IsType, std::type_identity<T>> Type{ std::type_identity<T>{} };
        inline static constexpr OptionalField<IsValue, decltype(A)> Value{ A };
    };

    namespace detail {
        template<typename T>
        inline constexpr auto makeParameterPackInfo() noexcept {
            return ParameterPackInfo<T, detail::ParameterPackInfoNull{}>{};
        }

        template<auto Value>
        inline constexpr auto makeParameterPackInfo() noexcept {
            return ParameterPackInfo<detail::ParameterPackInfoNull, Value>{};
        }
    }

    template<std::size_t Index, typename... Args>
    inline constexpr auto getPackItemAt() noexcept {
        return detail::makeParameterPackInfo<typename decltype(getTypeAt<Index, Args...>())::type>();
    }

    template<std::size_t Index, auto... Args>
    inline constexpr auto getPackItemAt() noexcept {
        return detail::makeParameterPackInfo<getValueAt<Index, Args...>()>();
    }

    template<std::size_t Index, typename One, auto Two>
    inline constexpr auto getPackItemAt() noexcept {
        static_assert(Index < 2, "Index is out of bounds!");
        if constexpr (Index == 0) return detail::makeParameterPackInfo<One>();
        else return detail::makeParameterPackInfo<Two>();
    }

    template<std::size_t Index, auto One, typename Two>
    inline constexpr auto getPackItemAt() noexcept {
        static_assert(Index < 2, "Index is out of bounds!");
        if constexpr (Index == 0) return detail::makeParameterPackInfo<One>();
        else return detail::makeParameterPackInfo<Two>();
    }
}

#endif