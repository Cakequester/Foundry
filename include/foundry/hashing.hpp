#ifndef FOUNDRY_HASHING_H_
#define FOUNDRY_HASHING_H_

#include <cstddef>
#include <cstdint>
#include <bit>

#include <string_view>

#include <array>

#include <type_traits>
#include <concepts>

#include <algorithm>
#include <utility>
#include <limits>

#include <foundry/integral.hpp>

namespace Foundry {
    class FNV1A {
    private:
        inline static constexpr std::uint64_t fnv_basis = 0xcbf29ce484222325ULL;
        inline static constexpr std::uint64_t fnv_prime = 0x100000001b3ULL;
    public:
        inline static constexpr std::uint64_t hash(std::string_view s, std::uint64_t seed = 0) noexcept {
            std::uint64_t h = fnv_basis ^ seed;
            for (char c : s) {
                h ^= static_cast<unsigned char>(c);
                h *= fnv_prime;
            }
            return h;
        }

        template<std::integral I>
        inline static constexpr std::uint64_t hash(I value, std::uint64_t seed = 0) noexcept {
            using U = std::make_unsigned_t<I>;
            const U v = static_cast<U>(value);
            std::uint64_t h = fnv_basis ^ seed;
            for (std::size_t i = 0; i < sizeof(U); ++i) {
                h ^= (v >> (8 * i)) & 0xffU;
                h *= fnv_prime;
            }
            return h;
        }

        template<std::integral I>
        inline static constexpr std::uint64_t hash(std::span<const I> values, std::uint64_t seed = 0) noexcept {
            std::uint64_t h = fnv_basis ^ seed;
            for (I value : values)
                h = fnv1a(value, h);
            return h;
        }
    };

    inline constexpr std::uint64_t hash_combine(std::uint64_t x, std::uint64_t y) noexcept {
        return x ^ (y + 0x9e3779b97f4a7c15ULL + (x << 6) + (x >> 2));
    }

    inline constexpr std::uint64_t mix_bits(std::uint64_t v) noexcept {
        v ^= v >> 30;
        v *= 0xbf58476d1ce4e5b9ULL;
        v ^= v >> 27;
        v *= 0x94d049bb133111ebULL;
        v ^= v >> 31;
        return v;
    }
}

#endif