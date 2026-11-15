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

// TODO: See if macros could reduce code re-use here.
namespace foundry {
    namespace detail {
        template<std::size_t I, typename Current, typename... Rest>
        inline constexpr auto get_type_at_impl() noexcept {
            if constexpr (I == 0) return std::type_identity<Current>{};
            else return get_type_at_impl<I - 1, Rest...>();
        }
    }

    template<std::size_t I, typename... Ts>
    inline constexpr auto get_type_at() noexcept requires(sizeof...(Ts) > 0) {
        static_assert(I < sizeof...(Ts), "Index is out of bounds.");
        return detail::get_type_at_impl<I, Ts...>();
    }

    template<typename T, typename... Ts>
    inline constexpr bool contains_type() noexcept {
        return (std::same_as<T, Ts> || ...);
    }

    template<typename T, typename... Ts>
    inline constexpr std::optional<std::size_t> index_of_type() noexcept {
        std::size_t i = 0;
        std::optional<std::size_t> result;
        (((std::same_as<T, Ts> && !result) ? (result = i, true) : (++i, false)) || ...);
        return result;
    }

    template<typename... Ts>
    class type_list {
    public:
        template<typename...>
        friend class type_list;

        template<std::size_t I>
        using at = typename decltype(get_type_at<I, Ts...>())::type;

        static constexpr auto front() noexcept requires(sizeof...(Ts) > 0) {
            return std::type_identity<at<0>>{};
        }

        static constexpr auto back() noexcept requires(sizeof...(Ts) > 0) {
            return std::type_identity<at<size() - 1>>{};
        }

        template<typename T>
        static constexpr bool contains() noexcept {
            return contains_type<T, Ts...>();
        }

        template<typename... Us>
        static constexpr bool starts_with() noexcept {
            if constexpr (sizeof...(Us) > sizeof...(Ts)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (std::same_as<at<I>, Us> && ...);
            }(std::index_sequence_for<Us...>{});
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::type_list>()>())
        static constexpr bool starts_with_list() noexcept {
            return[]<typename... Us>(deducible<type_list<Us...>>) {
                return starts_with<Us...>();
            }(deducible_of<Other>());
        }

        template<typename T>
        static constexpr std::optional<std::size_t> index_of() noexcept {
            return index_of_type<T, Ts...>();
        }

        static constexpr auto reverse() noexcept {
            return []<std::size_t... I>(std::index_sequence<I...>) {
                return type_list<at<sizeof...(Ts) - 1 - I>...>{};
            }(std::index_sequence_for<Ts...>{});
        }

        template<std::size_t Start, std::size_t End>
        static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Ts), "Invalid range.");
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return type_list<at<S + I>...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<typename... Us>
        static constexpr auto push_front() noexcept {
            return type_list<Us..., Ts...>{};
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::type_list>()>())
        static constexpr auto push_front_list() noexcept {
            return []<typename... Us>(deducible<type_list<Us...>>) {
                return type_list<Us..., Ts...>{};
            }(deducible_of<Other>());
        }

        template<typename... Us>
        static constexpr auto append() noexcept {
            return type_list<Ts..., Us...>{};
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::type_list>()>())
        inline static constexpr auto append_list() noexcept {
            return []<typename... Others>() {
                return type_list<Ts..., Others...>{};
            }();
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index = index_of<T>();

            if constexpr (!index) return type_list<Ts...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return type_list<at<(I < Skip ? I : I + 1)>...>{};
            }(cw<*index>, std::make_index_sequence<sizeof...(Ts) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Ts);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    namespace detail {
        template<std::size_t I, auto Current, auto... Rest>
        inline constexpr decltype(auto) get_value_at_impl() noexcept {
            if constexpr (I == 0) return Current;
            else return get_value_at_impl<I - 1, Rest...>();
        }
    }

    template<std::size_t I, auto... Values>
    inline constexpr decltype(auto) get_value_at() noexcept requires(sizeof...(Values) > 0) {
        static_assert(I < sizeof...(Values), "Index is out of bounds.");
        return detail::get_value_at_impl<I, Values...>();
    }

    template<auto Value, auto... Values>
    inline constexpr bool contains_value() noexcept {
        return ((are_values_equal<Value, Values>()) || ...);
    }

    template<auto Value, auto... Values>
    inline constexpr std::optional<std::size_t> index_of_value() noexcept {
        std::size_t i = 0;
        std::optional<std::size_t> result;
        (((are_values_equal<Value, Values>() && !result) ? (result = i, true) : (++i, false)) || ...);
        return result;
    }

    template<auto... Values>
    class value_list {
    private:
        // MSVC lambda bug. I have NO idea what causes this or if it's an existing bug, but without this you get C7746 on call sites.
        template<std::size_t... I>
        inline static constexpr auto reverse_impl(std::index_sequence<I...>) noexcept {
            return value_list<at<sizeof...(Values) - 1 - I>()...>{};
        }
    public:
        template<auto...>
        friend class value_list;

        template<std::size_t I>
        inline static constexpr decltype(auto) at() noexcept requires(sizeof...(Values) > 0) {
            static_assert(I < sizeof...(Values), "Index out of bounds");
            return get_value_at<I, Values...>();
        }

        inline static constexpr decltype(auto) front() noexcept requires(sizeof...(Values) > 0) {
            return at<0>();
        }

        inline static constexpr decltype(auto) back() noexcept requires(sizeof...(Values) > 0) {
            return at<size() - 1>();
        }

        template<auto Value>
        inline static constexpr bool contains() noexcept {
            return contains_value<Value, Values...>();
        }

        template<auto... Others>
        inline static constexpr bool starts_with() noexcept {
            if constexpr (sizeof...(Others) > sizeof...(Values)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (are_values_equal<at<I>(), Others>() && ...);
            }(std::make_index_sequence<sizeof...(Others)>{});
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::value_list>()>())
        static constexpr bool starts_with_list() noexcept {
            return []<auto... Others>(deducible<value_list<Others...>>) {
                return starts_with<Others...>();
            }(deducible_of<Other>());
        }

        template<auto Value>
        inline static constexpr std::optional<std::size_t> index_of() noexcept {
            return index_of_value<Value, Values...>();
        }

        inline static constexpr auto reverse() noexcept {
            return reverse_impl(std::make_index_sequence<sizeof...(Values)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Values), "Invalid range.");
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return value_list<at<S + I>()...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<auto... Others>
        inline static constexpr auto push_front() noexcept {
            return value_list<Others..., Values...>{};
        }

        template<auto... Others>
        inline static constexpr auto append() noexcept {
            return value_list<Values..., Others...>{};
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::value_list>()>())
        inline static constexpr auto append_list() noexcept {
            return []<auto... Others>(deducible<value_list<Others...>>) {
                return value_list<Values..., Others...>{};
            }(deducible_of<Other>());
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index = index_of<Value>();

            if constexpr (!index) return value_list<Values...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return value_list<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index>, std::make_index_sequence<sizeof...(Values) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Values);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    namespace detail {
        class mixed_template_null {};
    }

    template<typename T, auto Value>
    class mixed_template {
    public:
        inline static constexpr bool HasType = !std::same_as<T, detail::mixed_template_null>;
        inline static constexpr bool HasValue = !std::same_as<std::remove_cvref_t<decltype(Value)>, detail::mixed_template_null>;

        static_assert(HasType != HasValue, "Exactly one of a type or value must be supplied!");

        inline static constexpr decltype(auto) value() noexcept requires(HasValue) {
            return Value;
        }

        class type_holder {
        public:
            static_assert(HasType, "This mixed_template does not contain a type!");

            using type = T;
        };
    };

    template<typename T>
    inline constexpr auto make_mixed_template() noexcept {
        return mixed_template<T, detail::mixed_template_null{}>{};
    }

    template<auto Value>
    inline constexpr auto make_mixed_template() noexcept {
        return mixed_template<detail::mixed_template_null, Value>{};
    }

    // TODO: Fix code duplicate by turning certain lambdas into functions.
    template<auto... Templates>
        requires((meta::instance_of<decltype(Templates), meta::to_underlying_type<mixed_template>()>() && ...))
    class mixed_template_list {
    public:
        template<std::size_t I>
        inline static constexpr auto at() noexcept requires(sizeof...(Templates) > 0) {
            static_assert(I < sizeof...(Templates), "Index out of bounds");
            return get_value_at<I, Templates...>();
        }

        inline static constexpr auto front() noexcept requires(sizeof...(Templates) > 0) {
            return at<0>();
        }

        inline static constexpr auto back() noexcept requires(sizeof...(Templates) > 0) {
            return at<size() - 1>();
        }

        template<typename T>
        inline static constexpr bool contains() noexcept {
            return ([]<auto E>(constant_wrapper<E>) {
                if constexpr (E.HasType)
                    return std::same_as<typename decltype(E)::type_holder::type, T>;
                else
                    return false;
            }(cw<Templates>) || ...);
        }

        template<auto Value>
        inline static constexpr bool contains() noexcept {
            return ([]<auto E>(constant_wrapper<E>) {
                if constexpr (E.HasValue)
                    return are_values_equal<Value, E.value()>();
                else
                    return false;
            }(cw<Templates>) || ...);
        }

        template<auto... Others>
            requires((meta::instance_of<decltype(Others), meta::to_underlying_type<foundry::mixed_template>()>() && ...))
        inline static constexpr bool starts_with() noexcept {
            if constexpr (sizeof...(Others) > sizeof...(Templates)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (std::same_as<decltype(at<I>()), decltype(Others)> && ...);
            }(std::make_index_sequence<sizeof...(Others)>{});
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::mixed_template_list>()>())
        static constexpr bool starts_with_list() noexcept {
            return []<auto... Others>(deducible<mixed_template_list<Others...>>) {
                return starts_with<Others...>();
            }(deducible_of<Other>());
        }

        template<typename T>
        inline static constexpr std::optional<std::size_t> index_of() noexcept {
            std::size_t i = 0;
            std::optional<std::size_t> result;
            (((Templates.HasType && std::same_as<T, typename decltype(Templates)::type_holder::type>() && !result) ? (result = i, true) : (++i, false)) || ...);
            return result;
        }

        template<auto Value>
        inline static constexpr std::optional<std::size_t> index_of() noexcept {
            std::size_t i = 0;
            std::optional<std::size_t> result;
            (((Templates.HasValue && are_values_equal<Value, Templates.value()>() && !result) ? (result = i, true) : (++i, false)) || ...);
            return result;
        }

        inline static constexpr auto reverse() noexcept {
            return []<std::size_t... I>(std::index_sequence<I...>) {
                return mixed_template_list<at<sizeof...(Templates) - 1 - I>()...>{};
            }(std::make_index_sequence<sizeof...(Templates)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Templates), "Invalid range.");
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return mixed_template_list<at<S + I>()...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<typename... Others>
        inline static constexpr auto push_front() noexcept {
            return mixed_template_list<make_mixed_template<Others>()..., Templates...>{};
        }

        template<auto... Others>
        inline static constexpr auto push_front() noexcept {
            return mixed_template_list<make_mixed_template<Others>()..., Templates...>{};
        }

        template<typename... Others>
        inline static constexpr auto append() noexcept {
            return mixed_template_list<Templates..., make_mixed_template<Others>()...>{};
        }

        template<auto... Others>
        inline static constexpr auto append() noexcept {
            return mixed_template_list<Templates..., make_mixed_template<Others>()...>{};
        }

        template<typename Other>
            requires(meta::instance_of<Other, meta::to_underlying_type<foundry::mixed_template_list>()>())
        inline static constexpr auto append_list() noexcept {
            return []<auto... Others>(deducible<mixed_template_list<Others...>>) {
                return mixed_template_list<Templates..., Others...>{};
            }(deducible_of<Other>());
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index = index_of<T>();

            if constexpr (!index) return mixed_template_list<Templates...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return mixed_template_list<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index>, std::make_index_sequence<sizeof...(Templates) - 1>{});
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index = index_of<Value>();

            if constexpr (!index) return mixed_template_list<Templates...>{};
            else return[]<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return mixed_template_list<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index>, std::make_index_sequence<sizeof...(Templates) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Templates);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    template<auto... Templates>
        requires((meta::instance_of<decltype(Templates), meta::to_underlying_type<mixed_template>()>() && ...))
    class mixed_template_list_builder {
    public:
        template<typename... New>
            requires(sizeof...(New) > 0)
        inline static constexpr auto add() noexcept {
            return mixed_template_list_builder<Templates..., make_mixed_template<New>()...>{};
        }

        template<auto... New>
            requires(sizeof...(New) > 0)
        inline static constexpr auto add() noexcept {
            return mixed_template_list_builder<Templates..., make_mixed_template<New>()...>{};
        }

        inline static constexpr auto build() noexcept {
            return mixed_template_list<Templates...>{};
        }
    };
}

#endif