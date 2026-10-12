#ifndef FOUNDRY_COMPILE_TIME_H_
#define FOUNDRY_COMPILE_TIME_H_

#include <cstddef>
#include <cstdint>

#include <array>
#include <optional>

#include <string_view>

#include <type_traits>
#include <concepts>

#include <memory>
#include <utility>

#include <source_location>

#include <stdexcept>

namespace Foundry {
    namespace detail {
        template<template<typename...> class T, typename U>
        struct is_instance_of_helper : std::false_type {};

        template<template<typename...> class T, typename... Args>
        struct is_instance_of_helper<T, T<Args...>> : std::true_type {};

        template<template<auto...> class T, typename U>
        struct is_type_with_NTTPS_instance_of : std::false_type {};

        template<template<auto...> class T, auto... Args>
        struct is_type_with_NTTPS_instance_of<T, T<Args...>> : std::true_type {};
    }

    template<typename T, template<typename...> class U>
        requires(detail::is_instance_of_helper<U, T>::value)
    struct is_instance_of : std::true_type {};

    template<typename T, template<typename...> class U>
    inline constexpr bool is_instance_of_v = is_instance_of<T, U>::value;

    template<typename T, template<typename...> class U>
    inline constexpr bool instance_of() noexcept {
        return is_instance_of_v<T, U>;
    }

    template<typename T, template<auto...> class U>
    inline constexpr bool instance_of() noexcept {
        return detail::is_type_with_NTTPS_instance_of<U, T>::value;
    }

    template<typename Derived, template<typename...> class Base, typename... Args>
    concept derived_from_instance = requires(Derived *p) {
        []<typename... Args>(Base<Args...>*) {}(p);
    };

    template<typename T, typename... Rest>
    struct variadic_front {
        using type = T;
    };

    template<typename... Ts>
    using variadic_front_t = variadic_front<Ts...>::type;

    template<auto Constraint, typename... Ts>
    concept satisfies_all = (Constraint.template operator()<Ts>() && ...);

    template<std::floating_point T>
    struct matching_integral {
        using type = std::conditional_t<std::same_as<T, float>, std::uint32_t, std::uint64_t>;
    };

    template<typename T>
    using matching_integral_t = matching_integral<T>::type;

    template<std::size_t N>
    struct FixedString {
        char chars[N]{};

        inline constexpr FixedString(const char(&str)[N]) noexcept {
            for (std::size_t i = 0; i < N; ++i) chars[i] = str[i];
        }

        inline constexpr explicit FixedString(const char* str) noexcept {
            for (std::size_t i = 0; i < N; ++i) chars[i] = str[i];
        }

        inline constexpr std::string_view view() const noexcept {
            return std::string_view(chars, N - 1);
        }

        constexpr std::size_t size() const {
            return N - 1;
        }
    };

    template<FixedString S, FixedString Prefix>
    consteval bool string_starts_with() noexcept {
        if constexpr (Prefix.size() > S.size())
            return false;

        for (std::size_t i = 0; i < Prefix.size(); ++i)
            if (S.chars[i] != Prefix.chars[i])
                return false;

        return true;
    }

    template <typename Signature>
    class FunctionRef;

    template <typename R, typename... Args>
    class FunctionRef<R(Args...)> {
    private:
        void *_obj = nullptr;
        R(*_func)(void*, Args...) = nullptr;
    public:
        inline constexpr FunctionRef() noexcept = delete;
        inline constexpr FunctionRef(const FunctionRef&) noexcept = default;
        inline constexpr FunctionRef& operator=(const FunctionRef&) noexcept = default;

        template <typename F>
            requires (!std::same_as<std::decay_t<F>, FunctionRef>&& std::is_invocable_r_v<R, F, Args...>)
        inline constexpr FunctionRef(F&& f) noexcept {
            _obj = const_cast<void*>(static_cast<const void*>(std::addressof(f)));
            _func = [](void *ptr, Args... args) -> R {
                return (*static_cast<std::decay_t<F>>(ptr))(std::forward<Args>(args)...);
            };
        }

        inline R invoke(Args&&... args) const {
            return std::invoke(_func, _obj, std::forward<Args>(args)...);
        }

        inline R operator()(Args&&... args) const {
            return invoke(std::forward<Args>(args)...);
        }
    };

    class EmptyFieldType {};

    template<typename T>
    class OptionalField {
    public:
        inline static constexpr bool IsEmpty = false;

        T value;

        inline constexpr OptionalField(T value) noexcept : value(std::move(value)) {}

        inline constexpr operator T() noexcept {
            return value;
        }

        inline constexpr operator T() const noexcept {
            return value;
        }
    };

    template<>
    class OptionalField<EmptyFieldType> {
    public:
        inline static constexpr bool IsEmpty = true;
    };

    template<>
    class OptionalField<void> {
    public:
        inline static constexpr bool IsEmpty = true;
    };

    class EmptyCompileOptional {};
    inline constexpr auto null_optional = EmptyCompileOptional{};

    template<typename T>
    class CompileOptional {
    public:
        static_assert(!std::same_as<std::remove_cv_t<T>, std::nullopt_t> && !std::same_as<std::remove_cv_t<T>, std::in_place_t>,
            "T in CompileOptional<T> must not be nullopt_t or in_place_t");

        const std::array<T, 1> _storage;
        const bool _has_value = false;

        inline constexpr CompileOptional() noexcept(std::is_nothrow_default_constructible_v<T>) requires(std::is_default_constructible_v<T>) : _storage{ T() }, _has_value(true) {}
        inline constexpr CompileOptional(const CompileOptional&) noexcept = default;
        inline constexpr CompileOptional(CompileOptional&) noexcept = default;

        inline constexpr CompileOptional(EmptyCompileOptional) : _storage{}, _has_value(false) {}

        template<typename U>
        inline constexpr CompileOptional(U&& value) : _storage{ std::forward<U>(value) }, _has_value(true) {}

        inline constexpr T& value() const {
            if (!_has_value) throw std::bad_optional_access();
            return _storage.front();
        }

        inline constexpr bool has_value() const noexcept {
            return _has_value;
        }
    };

    template<typename T>
    class CompileOptional<T&> {
    public:
        T* const _storage = nullptr;

        constexpr CompileOptional() noexcept : _storage(nullptr) {}
        constexpr CompileOptional(const CompileOptional&) noexcept = default;
        constexpr CompileOptional(CompileOptional&&) noexcept = default;

        constexpr CompileOptional(EmptyCompileOptional) noexcept : _storage(nullptr) {}

        constexpr CompileOptional(T& value) noexcept : _storage(std::addressof(value)) {}
        constexpr CompileOptional(const T& value) noexcept : _storage(std::addressof(value)) {}

        constexpr T& value() const {
            if (!_storage) throw std::bad_optional_access();
            return *_storage;
        }

        constexpr bool has_value() const noexcept {
            return _storage != nullptr;
        }
    };

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
        template<typename...>
        friend class TypeList;

        template<typename... Others>
        using combine_rebind = TypeList<Others..., Ts...>;
    private:
        template<typename Current, typename... Rest>
        inline static constexpr auto reverseImplementation() noexcept {
            if constexpr (sizeof...(Rest) == 0) return TypeList<Current>{};
            else {
                auto reversed_rest = reverseImplementation<Rest...>();
                return reversed_rest.template appendRange<Current>();
            }
        }

        template<std::size_t Start, std::size_t End, std::size_t Index, typename Current, typename... Rest>
        inline static constexpr auto sublistImplementation() noexcept {
            constexpr std::size_t position = sizeof...(Ts) - Index;
            constexpr bool in_range = position >= Start && position < End;

            if constexpr (sizeof...(Rest) == 0) {
                if constexpr (in_range) return TypeList<Current>{};
                else return TypeList<>{};
            } else {
                auto rest = sublistImplementation<Start, End, Index - 1, Rest...>();
                if constexpr (in_range) return rest.template appendRange<Current>();
                else return rest;
            }
        }
    public:
        template<std::size_t Index>
        using at = typename decltype(getTypeAt<Index, Ts...>())::type;

        template<typename T>
        inline static constexpr bool contains() noexcept {
            return containsType<T, Ts...>();
        }

        template<typename T>
        inline static constexpr std::optional<std::size_t> indexOf() noexcept {
            return indexOfType<T, Ts...>();
        }

        inline static constexpr auto reverse() noexcept {
            if constexpr (sizeof...(Ts) < 1) return TypeList<>{};
            else return reverseImplementation<Ts...>();
        }

        template<std::size_t Start, std::size_t End>
        inline static constexpr auto sublist() noexcept {
            static_assert(Start <= End && End <= sizeof...(Ts), "Invalid range.");
            if constexpr (Start == End) return TypeList<>{};
            else return sublistImplementation<Start, End, sizeof...(Ts), Ts...>().reverse();
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
            requires(instance_of<TypeList, Other>())
        inline static constexpr auto append() noexcept {
            return typename Other::template combine_rebind<Ts...>{};
        }

        template<typename Other>
        inline static constexpr auto append() noexcept {
            return TypeList<Ts..., Other>{};
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
                using tail = decltype(sublist<index + 1, sizeof...(Ts)>());
                return sublist<0, index>().template append<tail>();
            }
        }

        inline static constexpr std::size_t size() noexcept {
            return sizeof...(Ts);
        }

        inline static constexpr bool empty() noexcept {
            return size() == 0;
        }
    };

    template <std::size_t Index, auto First, auto... Rest>
    inline static constexpr auto getValueAt() noexcept {
        if constexpr (Index == 0) return First;
        else return getValueAt<Index - 1, Rest...>();
    }


    namespace detail {
        template<typename U>
        static consteval const char* typeIdRawName() noexcept {
            return std::source_location::current().function_name();
        }
    }

    template<typename U>
    static consteval auto typeId() noexcept {
        constexpr const char* name = detail::typeIdRawName<U>();
        return FixedString<std::char_traits<char>::length(name) + 1>(name);
    }

    template<auto... Values>
    class ValueList {
    public:
        template<std::size_t Index>
        inline static constexpr decltype(auto) at() noexcept {
            static_assert(Index < sizeof...(Values), "Index out of bounds");
            return get_value<Index, Values...>();
        }
    };

    template<typename T>
    struct is_value_list : std::false_type {};

    template<template<auto...> class ListType, auto... Values>
    struct is_value_list<ListType<Values...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_value_list_v = is_value_list<T>::value;
};

#endif