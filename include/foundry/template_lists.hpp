#ifndef FOUNDRY_TEMPLATE_LISTS_H_
#define FOUNDRY_TEMPLATE_LISTS_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <utility>

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

        template<typename T>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            return indexOfType<T, Ts...>();
        }

        inline static constexpr auto reverse() noexcept {
            return[]<std::size_t... I>(std::index_sequence<I...>) {
                return TypeList<at<sizeof...(Ts) - 1 - I>...>{};
            }(std::make_index_sequence<sizeof...(Ts)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Ts), "Invalid range.");
            return[]<std::size_t... I>(std::index_sequence<I...>) {
                return TypeList<at<Start + I>...>{};
            }(std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<typename T>
        inline static constexpr auto pushFront() noexcept {
            return TypeList<T, Ts...>{};
        }

        template<typename Other>
        inline static constexpr auto append() noexcept {
            return TypeList<Ts..., Other>{};
        }

        template<typename Other>
            requires(instance_of<Other, Foundry::TypeList>())
        inline static constexpr auto appendList() noexcept {
            return[]<typename... Others>(TypeList<Others...>) {
                return TypeList<Ts..., Others...>{};
            }(Other{});
        }

        template<typename... Others>
        inline static constexpr auto appendRange() noexcept {
            return TypeList<Ts..., Others...>{};
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<T>();

            if constexpr (!index_of) return TypeList<Ts...>{};
            else {
                constexpr std::size_t index = *index_of;
                using head = decltype(sublist<0, index>());
                using tail = decltype(sublist<index + 1, sizeof...(Ts)>());
                return head::template appendList<tail>();
            }
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
        // MSVC: lambda form triggers C7746
        template<std::size_t... I>
        inline static constexpr auto reverseImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<sizeof...(Values) - 1 - I>()...>{};
        }

        template<std::size_t Start, std::size_t... I>
        inline static constexpr auto sublistImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<Start + I>()...>{};
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

        template<auto Other>
        inline static constexpr auto append() noexcept {
            return ValueList<Values..., Other>{};
        }

        template<typename Other>
            requires(instance_of<Other, Foundry::ValueList>())
        inline static constexpr auto appendList() noexcept {
            return[]<auto... Others>(ValueList<Others...>) {
                return ValueList<Values..., Others...>{};
            }(Other{});
        }

        template<auto... Others>
        inline static constexpr auto appendRange() noexcept {
            return ValueList<Values..., Others...>{};
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<Value>();

            if constexpr (!index_of) return ValueList<Values...>{};
            else {
                constexpr std::size_t index = *index_of;
                using head = decltype(sublist<0, index>());
                using tail = decltype(sublist<index + 1, sizeof...(Values)>());
                return head::template appendList<tail>();
            }
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Values);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };
}

#endif