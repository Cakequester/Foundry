#ifndef FOUNDRY_RANDOM_H_
#define FOUNDRY_RANDOM_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <source_location>

#include <foundry/hashing.hpp>
#include <foundry/compile_time.hpp>

namespace Foundry {
    class LCGUser {
    private:
        inline static constexpr std::uint64_t Multiplier = 6364136223846793005u;
        inline static constexpr std::uint64_t Increment = 1442695040888963407u;
    protected:
        inline static constexpr std::size_t randomizePermutationsFromProvidedLocation(const std::source_location& location) noexcept {
            const std::uint32_t line = location.line();
            const std::uint32_t column = location.column();

            return hash_combine(line, column);
        }

        inline static constexpr std::uint64_t permutate(std::size_t state, std::size_t permutations) noexcept {
            std::uint64_t acc_mul = 1, acc_add = 0;
            std::uint64_t cur_mul = Multiplier, cur_add = Increment;
            while (permutations) {
                if (permutations & 1) {
                    acc_mul *= cur_mul;
                    acc_add = acc_add * cur_mul + cur_add;
                }
                cur_add = (cur_mul + 1) * cur_add;
                cur_mul *= cur_mul;
                permutations >>= 1;
            }
            return acc_mul * state + acc_add;
        }
    };

    template<std::uint64_t Seed>
    class UnseededCompileRandom : public LCGUser {
    private:
    public:
        inline static constexpr std::uint64_t get(const std::source_location& location = std::source_location::current()) noexcept {
            return LCGUser::permutate(Seed, randomizePermutationsFromProvidedLocation(location));
        }

        inline static constexpr std::uint64_t get_in_range(std::uint64_t min, std::uint64_t max, const std::source_location& location = std::source_location::current()) noexcept {
            return min + get(location) % (max - min + 1);
        }
    };

    inline static constexpr std::uint64_t generateRandomFromTranslationUnit() noexcept {
        constexpr char date[] = __DATE__;
        constexpr char time[] = __TIME__;

        std::uint64_t hash = 0xcbf29ce484222325ULL;

        for (size_t i = 0; i < sizeof(date) - 1; ++i) {
            hash ^= static_cast<unsigned char>(date[i]);
            hash *= 0x100000001b3ULL;
        }

        for (size_t i = 0; i < sizeof(time) - 1; ++i) {
            hash ^= static_cast<unsigned char>(time[i]);
            hash *= 0x100000001b3ULL;
        }

        hash = FNV1A::hash(__FILE__, hash);

        return hash;
    }

    // Produces a value based on the type that is passed in.
    template<typename T>
    class TypeIdentifierRandom {
    private:
        template<typename U>
        inline static consteval auto type_identifier() noexcept {
            constexpr const char* identifier = std::source_location::current().function_name();
            return FixedString<std::char_traits<char>::length(identifier)>(identifier);
        }
        static_assert(type_identifier<int>().view() != type_identifier<float>().view(), "Your compiler does not support type identifiers!");
        static_assert(type_identifier<float>().view() == type_identifier<float>().view(), "Your compiler does not support type identifiers!");

        inline static constexpr std::uint64_t Hash = FNV1A::hash(type_identifier<T>().view());
    public:
        inline static constexpr std::uint64_t get() noexcept {
            return Hash;
        }
    };

    // Uses type and source location for extra entropy
    template<typename T>
    class MixedTypeIdentifierRandom : public LCGUser {
    public:
        inline static constexpr std::uint64_t get(const std::source_location& location = std::source_location::current()) noexcept {
            return LCGUser::permutate(TypeIdentifierRandom<T>::get(), LCGUser::randomizePermutationsFromProvidedLocation(location));
        }

        inline static constexpr std::uint64_t get_in_range(std::uint64_t min, std::uint64_t max, const std::source_location& location = std::source_location::current()) noexcept {
            return min + get(location) % (max - min + 1);
        }
    };
}

#endif