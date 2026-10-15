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
#include <algorithm>
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

    namespace detail {
        template<auto Value>
        struct are_values_equal_helper {};
    }

    template<auto A, auto B>
    inline constexpr bool are_values_equal() noexcept {
        return std::same_as<detail::are_values_equal_helper<A>, detail::are_values_equal_helper<B>>;
    }

    // TODO: Make first-class type with full suite of features. This is long overdue.
    template<std::size_t N>
    class FixedString {
    public:
        std::array<char, N> _chars{};

        inline constexpr FixedString() noexcept {}

        inline constexpr FixedString(const char(&str)[N]) noexcept {
            for (std::size_t i = 0; i < N; ++i) _chars[i] = str[i];
        }

        template<std::size_t M>
        inline constexpr bool operator==(const FixedString<M>& other) const noexcept {
            if constexpr (N != M) return false;
            else return std::equal(_chars.begin(), _chars.end(), other._chars.begin());
        }

        inline constexpr std::string_view view() const noexcept {
            return std::string_view(_chars);
        }

        inline constexpr std::size_t size() const noexcept {
            return N - 1;
        }
    };

    // TODO: Replace these two functions.
    template<std::size_t N>
    inline constexpr FixedString<N> makeFixedStringFromPointer(const char *str) {
        FixedString<N> result{};
        for (std::size_t i = 0; i < N; ++i) result._chars[i] = str[i];
        return result;
    }

    template<FixedString S, FixedString Prefix>
    inline constexpr bool string_starts_with() noexcept {
        if constexpr (Prefix.size() > S.size())
            return false;

        for (std::size_t i = 0; i < Prefix.size(); ++i)
            if (S._chars[i] != Prefix._chars[i])
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

    // TODO: Remember to add a backport of std::constant_wrapper!

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
};

#endif