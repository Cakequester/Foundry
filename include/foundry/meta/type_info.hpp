#ifndef FOUNDRY_TYPE_INFO_H_
#define FOUNDRY_TYPE_INFO_H_

#include <cstddef>
#include <cstdint>

#include <string_view>

#include <type_traits>
#include <concepts>

#include <source_location>

#include <foundry/meta/underlying_type.hpp>

#include <foundry/compile_time.hpp>

namespace Foundry::Meta {
    template<typename T, auto U>
        requires(std::derived_from<decltype(U), detail::UnderlyingTypeBase>)
    inline constexpr bool instance_of() noexcept {
        return std::same_as<decltype(extractUnderlyingType<T>()), decltype(U)>;
    }
}

#include <foundry/meta/template_info.hpp>
#include <foundry/template_lists.hpp>

namespace Foundry::Meta {
    template<typename T, auto U, typename... Args>
        requires(sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return getTemplateInfo<T>().templates.startsWith<makeMixedTemplate<Args>()...>();
    }

    template<typename T, auto U, auto... Args>
        requires(sizeof...(Args) > 0)
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return getTemplateInfo<T>().templates.startsWith<makeMixedTemplate<Args>()...>();
    }

    template<typename T, auto U, typename One, auto Two>
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return getTemplateInfo<T>().templates.startsWith<makeMixedTemplate<One>(), makeMixedTemplate<Two>()>();
    }

    template<typename T, auto U, auto One, typename Two>
    inline constexpr bool instance_of() noexcept {
        if constexpr (!instance_of<T, U>()) return false;
        else
            return getTemplateInfo<T>().templates.startsWith<makeMixedTemplate<One>(), makeMixedTemplate<Two>()>();
    }
}

#include <foundry/hashing.hpp>
#include <foundry/integral.hpp>

namespace Foundry::Meta {
    namespace detail {
        template<typename T>
        inline constexpr std::string_view getConcealedTypeName() noexcept {
            return std::source_location::current().function_name();
        }
    }

    class TypeIdentifier {
    private:
        uint128_t _identifier;

        inline constexpr TypeIdentifier(uint128_t identifier) noexcept : _identifier(identifier) {}
    public:
        template<typename T>
        inline static constexpr TypeIdentifier of() noexcept {
            static_assert(detail::getConcealedTypeName<double>() == detail::getConcealedTypeName<double>(), "Type identifiers are not supported on your compiler!");
            static_assert(detail::getConcealedTypeName<int>() != detail::getConcealedTypeName<float>(), "Type identifiers are not supported on your compiler!");

            return TypeIdentifier(Murmur128::hash(detail::getConcealedTypeName<std::remove_cvref_t<T>>()));
        }

        inline constexpr TypeIdentifier& operator=(const TypeIdentifier& rhs) noexcept {
            _identifier = rhs._identifier;
            return *this;
        }

        inline constexpr bool operator==(const TypeIdentifier& rhs) const noexcept {
            return _identifier == rhs._identifier;
        }

        inline constexpr std::strong_ordering operator<=>(const TypeIdentifier& rhs) const noexcept = delete;

        inline constexpr uint128_t asInt() const noexcept {
            return _identifier;
        }
    };

    template<typename T>
    inline constexpr TypeIdentifier typeId() noexcept {
        return TypeIdentifier::of<T>();
    }

    template <typename T>
    concept is_structural = requires {
        [] <T>() {};
    };
}

#endif