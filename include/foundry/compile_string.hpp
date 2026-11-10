#ifndef FOUNDRY_COMPILE_STRING_H_
#define FOUNDRY_COMPILE_STRING_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <utility>
#include <algorithm>

#include <stdexcept>

#include <foundry/meta/type_info.hpp>

namespace Foundry {
    template<character CharT, std::size_t N>
    class CompileGenericString {
    public:
        std::array<CharT, N> _chars{};

        inline constexpr CompileGenericString() noexcept {}

        inline constexpr CompileGenericString(const CharT(&str)[N]) noexcept {
            for (std::size_t i = 0; i < N; ++i) _chars[i] = str[i];
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr bool operator==(const Other& rhs) const noexcept {
            if constexpr (size() != Other::size()) return false;
            else return std::equal(_chars.begin(), _chars.end(), rhs._chars.begin());
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr std::strong_ordering operator<=>(const Other& rhs) const noexcept {
            if constexpr (size() != Other::size()) return false;
            else return std::lexicographical_compare_three_way(
                _chars.begin(), _chars.end(),
                rhs._chars.begin(), rhs._chars.end());
        }

        inline constexpr CharT& operator[](std::size_t pos) noexcept {
            return _chars[pos];
        }

        inline constexpr const CharT& operator[](std::size_t pos) const noexcept {
            return _chars[pos];
        }

        inline constexpr CharT& at(std::size_t pos) {
            if (pos > _chars.size()) throw std::out_of_range();
            return _chars[pos];
        }

        inline constexpr const CharT& at(std::size_t pos) const {
            if (pos > _chars.size()) throw std::out_of_range();

            return _chars[pos];
        }

        inline constexpr bool startsWith(CharT character) const noexcept {
            if constexpr (size() <= 0) return false;
            else
                return _chars[0] == character;
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr bool startsWith(const Other& other) const noexcept {
            if constexpr (size() < Other::size()) return false;
            else
                return std::equal(_chars.begin(), _chars.begin() + Other::size(), other._chars.begin());
        }

        inline constexpr bool endsWith(CharT character) const noexcept {
            if constexpr (size() <= 0) return false;
            else
                return _chars[size()] == character;
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr bool endsWith(const Other& other) const noexcept {
            if constexpr (size() < Other::size()) return false;
            else
                return std::equal(_chars.begin() + (size() - Other::size()), _chars.end(), other._chars.begin());
        }

        inline constexpr auto insert(std::size_t index, CharT character) const {
            if (index > _chars.size()) throw std::out_of_range();

            CompileGenericString<CharT, N + 1> result;

            std::copy_n(_chars.begin(), index, result.begin());
            *(result.begin() + index) = character;
            std::copy(_chars.begin() + index, _chars.end(), result.begin() + index + 1);

            return result;
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr auto insert(std::size_t index, const Other& other) const {
            if (index > _chars.size()) throw std::out_of_range();

            CompileGenericString<CharT, N + Other::size()> result;

            std::copy_n(_chars.begin(), index, result.begin());
            std::copy_n(other.begin(), Other::size(), result.begin() + index);
            std::copy(_chars.begin() + index, _chars.end(), result.begin() + index + Other::size());

            return result;
        }

        inline constexpr auto pushBack(CharT character) const {
            return insert(size(), character);
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr auto pushBack(const Other& other) const {
            return insert(size(), other);
        }

        inline constexpr auto append(CharT character) const {
            return insert(size(), character);
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr auto append(const Other& other) const {
            return insert(size(), other);
        }

        inline constexpr auto operator+(CharT rhs) const {
            return insert(size(), rhs);
        }

        template<typename Other>
            requires(Meta::instance_of<Other, CompileGenericString, CharT>())
        inline constexpr auto operator+(const Other& rhs) const {
            return insert(size(), rhs);
        }

        inline constexpr auto begin() noexcept {
            return _chars.begin();
        }
        
        inline constexpr auto cbegin() const noexcept {
            return _chars.cbegin();
        }

        inline constexpr auto begin() const noexcept {
            return cbegin();
        }

        inline constexpr auto end() noexcept {
            return _chars.end();
        }

        inline constexpr auto cend() const noexcept {
            return _chars.cend();
        }

        inline constexpr auto end() const noexcept {
            return cend();
        }

        inline constexpr auto rbegin() noexcept {
            return _chars.rbegin();
        }

        inline constexpr auto crbegin() const noexcept {
            return _chars.crbegin();
        }

        inline constexpr auto rbegin() const noexcept {
            return crbegin();
        }

        inline constexpr auto rend() noexcept {
            return _chars.rend();
        }

        inline constexpr auto crend() const noexcept {
            return _chars.crend();
        }

        inline constexpr auto rend() const noexcept {
            return crend();
        }

        inline constexpr bool empty() const noexcept {
            return _chars.empty();
        }

        inline static constexpr std::size_t size() noexcept {
            return N - 1;
        }

        inline static constexpr std::size_t length() noexcept {
            return N - 1;
        }
    };

    template<std::size_t N>
    using CompileString = CompileGenericString<char, N>;

    template<std::size_t N>
    using CompileWideString = CompileGenericString<wchar_t, N>;
}

#endif