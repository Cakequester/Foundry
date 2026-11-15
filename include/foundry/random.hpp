#ifndef FOUNDRY_RANDOM_H_
#define FOUNDRY_RANDOM_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>
#include <concepts>

#include <source_location>

#include <foundry/hashing.hpp>
#include <foundry/integral.hpp>
#include <foundry/compile_time.hpp>

namespace foundry {
    class lcg_user {
    private:
        inline static constexpr std::uint64_t Multiplier = 6364136223846793005u;
        inline static constexpr std::uint64_t Increment = 1442695040888963407u;
    protected:
        inline static constexpr std::size_t randomize_permutations_from_provided_location(const std::source_location& location) noexcept {
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
    class unseeded_compile_random : public lcg_user {
    private:
    public:
        inline static constexpr std::uint64_t get(const std::source_location& location = std::source_location::current()) noexcept {
            return lcg_user::permutate(Seed, randomize_permutations_from_provided_location(location));
        }

        inline static constexpr std::uint64_t get_in_range(std::uint64_t min, std::uint64_t max, const std::source_location& location = std::source_location::current()) noexcept {
            return min + get(location) % (max - min + 1);
        }
    };

    // Produces a value based on the type that is passed in.
    template<typename T>
    class type_identifier_random {
    private:
        inline static constexpr uint128_t Hash = meta::type_id<T>().as_int();
    public:
        inline static constexpr uint128_t get() noexcept {
            return Hash;
        }
    };

    // Uses type and source location for extra entropy
    template<typename T>
    class mixed_type_identifier_random : public lcg_user {
    public:
        inline static constexpr std::uint64_t get(const std::source_location& location = std::source_location::current()) noexcept {
            return lcg_user::permutate(type_identifier_random<T>::get(), randomize_permutations_from_provided_location(location));
        }

        inline static constexpr std::uint64_t get_in_range(std::uint64_t min, std::uint64_t max, const std::source_location& location = std::source_location::current()) noexcept {
            return min + get(location) % (max - min + 1);
        }
    };
}

#endif