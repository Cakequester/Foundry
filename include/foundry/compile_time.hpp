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

    template <typename T>
    concept character = std::is_same_v<std::decay_t<T>, char> ||
        std::is_same_v<std::decay_t<T>, signed char> ||
        std::is_same_v<std::decay_t<T>, unsigned char> ||
        std::is_same_v<std::decay_t<T>, wchar_t> ||
        std::is_same_v<std::decay_t<T>, char8_t> ||
        std::is_same_v<std::decay_t<T>, char16_t> ||
        std::is_same_v<std::decay_t<T>, char32_t>;

    namespace detail {
        template<auto Value>
        struct are_values_equal_helper {};
    }

    template<auto A, auto B>
    inline constexpr bool are_values_equal() noexcept {
        return std::same_as<detail::are_values_equal_helper<A>, detail::are_values_equal_helper<B>>;
    }

    template<std::size_t N, typename Func>
        requires(std::invocable<Func>)
    inline constexpr void constexpr_for(Func&& func) {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (func.template operator()<I>(), ...);
        }(std::make_index_sequence<N>{});
    }

    template<typename T>
    class deducible {
    public:
        inline constexpr deducible() noexcept {}
    };

    template<typename T>
    inline constexpr deducible<T> deducible_of() noexcept {
        return deducible<T>();
    }

    // TODO: Add operator lifting.
    template<auto Value>
    class constant_wrapper {
    public:
        using type = decltype(Value);
        inline static constexpr type value = Value;
    };

    template<auto Value>
    inline constexpr constant_wrapper<Value> cw{};

    template<bool Condition, typename T>
    class OptionalField {
    public:
        inline static constexpr bool IsEmpty = !Condition;

        T value;

        inline constexpr OptionalField(T value) noexcept : value(std::move(value)) {}

        inline constexpr OptionalField& operator=(T value) noexcept {
            this->value = value;
            return *this;
        }

        inline constexpr operator T() noexcept {
            return value;
        }

        inline constexpr operator T() const noexcept {
            return value;
        }
    };

    template<typename T>
    class OptionalField<false, T> {
    public:
        inline static constexpr bool IsEmpty = true;

        inline constexpr OptionalField(auto&&...) noexcept {}
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
};

#endif