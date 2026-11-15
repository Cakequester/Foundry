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

namespace foundry {
    template<typename T, typename... Rest>
    struct variadic_front {
        using type = T;
    };

    template<typename... Ts>
    using variadic_front_t = variadic_front<Ts...>::type;

    template<typename... Ts>
    concept identical_types = (std::same_as<typename variadic_front<Ts...>::type, Ts> && ...);

    template<auto Constraint, typename... Ts>
    concept satisfies_all = (Constraint.template operator()<Ts>() && ...);

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
    class optional_field {
    public:
        inline static constexpr bool IsEmpty = !Condition;

        T value;

        inline constexpr optional_field(T value) noexcept : value(std::move(value)) {}

        inline constexpr optional_field& operator=(T value) noexcept {
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
    class optional_field<false, T> {
    public:
        inline static constexpr bool IsEmpty = true;

        constexpr optional_field(auto&&...) noexcept {}
    };

    class empty_compile_optional {};
    inline constexpr auto null_optional = empty_compile_optional{};

    template<typename T>
    class compile_optional {
    public:
        static_assert(!std::same_as<std::remove_cv_t<T>, std::nullopt_t> && !std::same_as<std::remove_cv_t<T>, std::in_place_t>,
            "T in CompileOptional<T> must not be nullopt_t or in_place_t");

        const std::array<T, 1> _storage;
        const bool _has_value = false;

        constexpr compile_optional() noexcept(std::is_nothrow_default_constructible_v<T>) requires(std::is_default_constructible_v<T>) : _storage{ T() }, _has_value(true) {}
        constexpr compile_optional(const compile_optional&) noexcept = default;
        constexpr compile_optional(compile_optional&) noexcept = default;

        constexpr compile_optional(empty_compile_optional) : _storage{}, _has_value(false) {}

        template<typename U>
        constexpr compile_optional(U&& value) : _storage{ std::forward<U>(value) }, _has_value(true) {}

        constexpr T& value() const {
            if (!_has_value) throw std::bad_optional_access();
            return _storage.front();
        }

        constexpr bool has_value() const noexcept {
            return _has_value;
        }
    };

    template<typename T>
    class compile_optional<T&> {
    public:
        T* const _storage = nullptr;

        constexpr compile_optional() noexcept : _storage(nullptr) {}
        constexpr compile_optional(const compile_optional&) noexcept = default;
        constexpr compile_optional(compile_optional&&) noexcept = default;

        constexpr compile_optional(empty_compile_optional) noexcept : _storage(nullptr) {}

        constexpr compile_optional(T& value) noexcept : _storage(std::addressof(value)) {}
        constexpr compile_optional(const T& value) noexcept : _storage(std::addressof(value)) {}

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