#ifndef FOUNDRY_HASHING_H_
#define FOUNDRY_HASHING_H_

#include <cstdint>

#include <span>

#include <string_view>

#include <type_traits>
#include <concepts>

#include <foundry/integral.hpp>

namespace Foundry {
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

    inline static constexpr std::uint64_t rotl(std::uint64_t x, int r) noexcept {
        return (x << r) | (x >> (64 - r));
    }

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

    class Murmur128 {
    private:
        inline static constexpr std::uint64_t c1 = 0x87c37b91114253d5ULL;
        inline static constexpr std::uint64_t c2 = 0x4cf5ad432745937fULL;

        inline static constexpr std::uint64_t fmix(std::uint64_t k) noexcept {
            k ^= k >> 33;
            k *= 0xff51afd7ed558ccdULL;
            k ^= k >> 33;
            k *= 0xc4ceb9fe1a85ec53ULL;
            k ^= k >> 33;
            return k;
        }

        inline static constexpr std::uint64_t load(std::string_view s, std::size_t offset) noexcept {
            std::uint64_t value = 0;

            for (int i = 0; i < 8; ++i) {
                value |= std::uint64_t(
                    static_cast<unsigned char>(s[offset + i])
                ) << (i * 8);
            }

            return value;
        }
    public:
        inline static constexpr uint128_t hash(std::string_view s, std::uint64_t seed = 0) noexcept {
            const std::size_t len = s.size();
            const std::size_t nblocks = len / 16;

            std::uint64_t h1 = seed;
            std::uint64_t h2 = seed;

            // Body
            for (std::size_t i = 0; i < nblocks; ++i) {
                const std::size_t offset = i * 16;

                std::uint64_t k1 = load(s, offset);
                std::uint64_t k2 = load(s, offset + 8);

                k1 *= c1;
                k1 = rotl(k1, 31);
                k1 *= c2;
                h1 ^= k1;

                h1 = rotl(h1, 27);
                h1 += h2;
                h1 = h1 * 5 + 0x52dce729;

                k2 *= c2;
                k2 = rotl(k2, 33);
                k2 *= c1;
                h2 ^= k2;

                h2 = rotl(h2, 31);
                h2 += h1;
                h2 = h2 * 5 + 0x38495ab5;
            }

            // Tail
            const std::size_t tail_offset = nblocks * 16;
            const std::size_t remaining = len & 15;

            std::uint64_t k1 = 0;
            std::uint64_t k2 = 0;

            for (std::size_t i = 8; i < remaining; ++i) {
                k2 |= std::uint64_t(
                    static_cast<unsigned char>(s[tail_offset + i])
                ) << ((i - 8) * 8);
            }

            if (remaining > 8) {
                k2 *= c2;
                k2 = rotl(k2, 33);
                k2 *= c1;
                h2 ^= k2;
            }

            for (std::size_t i = 0; i < remaining && i < 8; ++i) {
                k1 |= std::uint64_t(
                    static_cast<unsigned char>(s[tail_offset + i])
                ) << (i * 8);
            }

            if (remaining > 0) {
                k1 *= c1;
                k1 = rotl(k1, 31);
                k1 *= c2;
                h1 ^= k1;
            }

            // Finalization
            h1 ^= static_cast<std::uint64_t>(len);
            h2 ^= static_cast<std::uint64_t>(len);

            h1 += h2;
            h2 += h1;

            h1 = fmix(h1);
            h2 = fmix(h2);

            h1 += h2;
            h2 += h1;

            return (static_cast<uint128_t>(h2) << 64) | h1;
        }
    };
}

#endif