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
namespace Foundry {
    namespace detail {
        template<std::size_t I, typename Current, typename... Rest>
        inline constexpr auto getTypeAtImpl() noexcept {
            if constexpr (I == 0) return std::type_identity<Current>{};
            else return getTypeAtImpl<I - 1, Rest...>();
        }
    }

    template<std::size_t I, typename... Ts>
    inline constexpr auto getTypeAt() noexcept requires(sizeof...(Ts) > 0) {
        static_assert(I < sizeof...(Ts), "Index is out of bounds.");
        return detail::getTypeAtImpl<I, Ts...>();
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

        template<std::size_t I>
        using at = typename decltype(getTypeAt<I, Ts...>())::type;

        inline static constexpr auto front() noexcept requires(sizeof...(Ts) > 0) {
            return std::type_identity<at<0>>{};
        }

        inline static constexpr auto back() noexcept requires(sizeof...(Ts) > 0) {
            return std::type_identity<at<size() - 1>>{};
        }

        template<typename T>
        inline static constexpr bool contains() noexcept {
            return containsType<T, Ts...>();
        }

        inline static constexpr bool contains() noexcept {
        
        }

        template<typename... Us>
        inline static constexpr bool startsWith() noexcept {
            if constexpr (sizeof...(Us) > sizeof...(Ts)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (std::same_as<at<I>, Us> && ...);
            }(std::index_sequence_for<Us...>{});
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<TypeList>()>())
        static constexpr bool startsWithList() noexcept {
            return[]<typename... Us>(deducible<TypeList<Us...>>) {
                return startsWith<Us...>();
            }(deducible_of<Other>());
        }

        template<typename T>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            return indexOfType<T, Ts...>();
        }

        inline static constexpr auto reverse() noexcept {
            return []<std::size_t... I>(std::index_sequence<I...>) {
                return TypeList<at<sizeof...(Ts) - 1 - I>...>{};
            }(std::index_sequence_for<Ts...>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Ts), "Invalid range.");
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return TypeList<at<S + I>...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
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
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<Foundry::TypeList>()>())
        inline static constexpr auto appendList() noexcept {
            return []<typename... Others>() {
                return TypeList<Ts..., Others...>{};
            }();
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<T>();

            if constexpr (!index_of) return TypeList<Ts...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return TypeList<at<(I < Skip ? I : I + 1)>...>{};
            }(cw<*index_of>, std::make_index_sequence<sizeof...(Ts) - 1>{});
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
        inline constexpr decltype(auto) getValueAtImpl() noexcept {
            if constexpr (I == 0) return Current;
            else return getValueAtImpl<I - 1, Rest...>();
        }
    }

    template<std::size_t I, auto... Values>
    inline constexpr decltype(auto) getValueAt() noexcept requires(sizeof...(Values) > 0) {
        static_assert(I < sizeof...(Values), "Index is out of bounds.");
        return detail::getValueAtImpl<I, Values...>();
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
        // MSVC lambda bug. I have NO idea what causes this or if it's an existing bug, but without this you get C7746.
        template<std::size_t... I>
        inline static constexpr auto reverseImpl(std::index_sequence<I...>) noexcept {
            return ValueList<at<sizeof...(Values) - 1 - I>()...>{};
        }
    public:
        template<auto...>
        friend class ValueList;

        template<std::size_t I>
        inline static constexpr decltype(auto) at() noexcept requires(sizeof...(Values) > 0) {
            static_assert(I < sizeof...(Values), "Index out of bounds");
            return getValueAt<I, Values...>();
        }

        inline static constexpr decltype(auto) front() noexcept requires(sizeof...(Values) > 0) {
            return at<0>();
        }

        inline static constexpr decltype(auto) back() noexcept requires(sizeof...(Values) > 0) {
            return at<size() - 1>();
        }

        template<auto Value>
        inline static constexpr bool contains() noexcept {
            return containsValue<Value, Values...>();
        }

        template<auto... Others>
        inline static constexpr bool startsWith() noexcept {
            if constexpr (sizeof...(Others) > sizeof...(Values)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (are_values_equal<at<I>(), Others>() && ...);
            }(std::make_index_sequence<sizeof...(Others)>{});
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<ValueList>()>())
        static constexpr bool startsWithList() noexcept {
            return []<auto... Others>(deducible<ValueList<Others...>>) {
                return startsWith<Others...>();
            }(deducible_of<Other>());
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
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return ValueList<at<S + I>()...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
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
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<Foundry::ValueList>()>())
        inline static constexpr auto appendList() noexcept {
            return []<auto... Others>(deducible<ValueList<Others...>>) {
                return ValueList<Values..., Others...>{};
            }(deducible_of<Other>());
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<Value>();

            if constexpr (!index_of) return ValueList<Values...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return ValueList<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index_of>, std::make_index_sequence<sizeof...(Values) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Values);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    namespace detail {
        class MixedTemplateNull {};
    }

    template<typename T, auto Value>
    class MixedTemplate {
    public:
        inline static constexpr bool HasType = !std::same_as<T, detail::MixedTemplateNull>;
        inline static constexpr bool HasValue = !std::same_as<std::remove_cvref_t<decltype(Value)>, detail::MixedTemplateNull>;

        static_assert(HasType != HasValue, "Exactly one of a type or value must be supplied!");

        inline static constexpr decltype(auto) value() noexcept requires(HasValue) {
            return Value;
        }

        class type_holder {
        public:
            static_assert(HasType, "This MixedTemplate does not contain a type!");

            using type = T;
        };
    };

    template<typename T>
    inline constexpr auto makeMixedTemplate() noexcept {
        return MixedTemplate<T, detail::MixedTemplateNull{}>{};
    }

    template<auto Value>
    inline constexpr auto makeMixedTemplate() noexcept {
        return MixedTemplate<detail::MixedTemplateNull, Value>{};
    }

    // TODO: Fix code duplicate by turning certain lambdas into functions.
    template<auto... Templates>
        requires((Meta::instance_of<decltype(Templates), Meta::toUnderlyingType<MixedTemplate>()>() && ...))
    class MixedTemplateList {
    public:
        template<std::size_t I>
        inline static constexpr auto at() noexcept requires(sizeof...(Templates) > 0) {
            static_assert(I < sizeof...(Templates), "Index out of bounds");
            return getValueAt<I, Templates...>();
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
            requires((Meta::instance_of<decltype(Others), Meta::toUnderlyingType<Foundry::MixedTemplate>()>() && ...))
        inline static constexpr bool startsWith() noexcept {
            if constexpr (sizeof...(Others) > sizeof...(Templates)) return false;
            else return []<std::size_t... I>(std::index_sequence<I...>) {
                return (std::same_as<decltype(at<I>()), decltype(Others)> && ...);
            }(std::make_index_sequence<sizeof...(Others)>{});
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<Foundry::MixedTemplateList>()>())
        static constexpr bool startsWithList() noexcept {
            return []<auto... Others>(deducible<MixedTemplateList<Others...>>) {
                return startsWith<Others...>();
            }(deducible_of<Other>());
        }

        template<typename T>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            std::size_t i = 0;
            std::optional<std::size_t> result;
            (((Templates.HasType && std::same_as<T, typename decltype(Templates)::type_holder::type>() && !result) ? (result = i, true) : (++i, false)) || ...);
            return result;
        }

        template<auto Value>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            std::size_t i = 0;
            std::optional<std::size_t> result;
            (((Templates.HasValue && are_values_equal<Value, Templates.value()>() && !result) ? (result = i, true) : (++i, false)) || ...);
            return result;
        }

        inline static constexpr auto reverse() noexcept {
            return []<std::size_t... I>(std::index_sequence<I...>) {
                return MixedTemplateList<at<sizeof...(Templates) - 1 - I>()...>{};
            }(std::make_index_sequence<sizeof...(Templates)>{});
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Templates), "Invalid range.");
            return []<std::size_t S, std::size_t... I>(constant_wrapper<S>, std::index_sequence<I...>) {
                return MixedTemplateList<at<S + I>()...>{};
            }(cw<Start>, std::make_index_sequence<End - Start>{});
        }

        template<std::size_t End>
        inline static constexpr auto sublist() noexcept {
            return sublist<0, End>();
        }

        template<typename T>
        inline static constexpr auto pushFront() noexcept {
            return MixedTemplateList<makeMixedTemplate<T>(), Templates...>{};
        }

        template<auto Value>
        inline static constexpr auto pushFront() noexcept {
            return MixedTemplateList<makeMixedTemplate<Value>(), Templates...>{};
        }

        template<typename... Others>
        inline static constexpr auto append() noexcept {
            return MixedTemplateList<Templates..., makeMixedTemplate<Others>()...>{};
        }

        template<auto... Others>
        inline static constexpr auto append() noexcept {
            return MixedTemplateList<Templates..., makeMixedTemplate<Others>()...>{};
        }

        template<typename Other>
            requires(Meta::instance_of<Other, Meta::toUnderlyingType<Foundry::MixedTemplateList>()>())
        inline static constexpr auto appendList() noexcept {
            return []<auto... Others>(deducible<MixedTemplateList<Others...>>) {
                return MixedTemplateList<Templates..., Others...>{};
            }(deducible_of<Other>());
        }

        template<typename T>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<T>();

            if constexpr (!index_of) return MixedTemplateList<Templates...>{};
            else return []<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return MixedTemplateList<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index_of>, std::make_index_sequence<sizeof...(Templates) - 1>{});
        }

        template<auto Value>
        inline static constexpr auto remove() noexcept {
            constexpr std::optional<std::size_t> index_of = indexOf<Value>();

            if constexpr (!index_of) return MixedTemplateList<Templates...>{};
            else return[]<std::size_t Skip, std::size_t... I>(constant_wrapper<Skip>, std::index_sequence<I...>) {
                return MixedTemplateList<at<(I < Skip ? I : I + 1)>()...>{};
            }(cw<*index_of>, std::make_index_sequence<sizeof...(Templates) - 1>{});
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Templates);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    template<auto... Templates>
        requires((Meta::instance_of<decltype(Templates), Meta::toUnderlyingType<MixedTemplate>()>() && ...))
    class MixedTemplateListBuilder {
    public:
        template<typename... New>
            requires(sizeof...(New) > 0)
        inline static constexpr auto add() noexcept {
            return MixedTemplateListBuilder<Templates..., makeMixedTemplate<New>()...>{};
        }

        template<auto... New>
            requires(sizeof...(New) > 0)
        inline static constexpr auto add() noexcept {
            return MixedTemplateListBuilder<Templates..., makeMixedTemplate<New>()...>{};
        }

        inline static constexpr auto build() noexcept {
            return MixedTemplateList<Templates...>{};
        }
    };
}

#endif