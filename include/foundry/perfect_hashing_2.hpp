#ifndef FOUNDRY_PERFECT_HASHING_H_
#define FOUNDRY_PERFECT_HASHING_H_

#include <cstddef>
#include <cstdint>

#include <array>
#include <tuple>

#include <type_traits>
#include <concepts>

#include <utility>
#include <algorithm>
#include <numeric>

#include <foundry/meta/type_info.hpp>

#include <foundry/hashing.hpp>
#include <foundry/random.hpp>

namespace Foundry {
    namespace detail {
        template<typename Storable, std::size_t TableSize>
        class PerfectHasherStorage {
        public:
            inline static constexpr std::size_t BucketCount = calculateBucketCount<TableSize>();

            std::array<Storable, TableSize> _entries;
            std::array<std::size_t, TableSize> _table{};
            std::array<std::size_t, BucketCount> _displacements{};
        };

        template<std::size_t TableSize>
        inline consteval std::size_t calculateBucketCount() noexcept {
            return TableSize == 0 ? 1 : TableSize;
        }

        template<typename Storable, std::size_t TableSize, bool NTTP = false>
        class PerfectHasherStructurer {
        public:
            using Storage = PerfectHasherStorage<Storable, TableSize>;
        protected:
            const Storage _storage;

            inline constexpr PerfectHasherHelper(Storage&& storage) noexcept : _storage(storage) {}
        };

        template<typename Storable, std::size_t TableSize>
        class PerfectHasherStructurer<Storable, TableSize, true> {
        public:
            using Storage = PerfectHasherStorage<Storable, TableSize>;
            const Storage _storage;
        protected:
            inline constexpr PerfectHasherHelper(Storage&& storage) noexcept : _storage(storage) {}
        };

        class KeyExtractor {};

        class PairFirstExtractor : public KeyExtractor {
        public:
            template<typename P>
                requires(Meta::instance_of<P, std::pair>())
            inline static constexpr auto get(const P& p) noexcept { return p.first; }
        };

        class IdentityExtractor : public KeyExtractor {
        public:
            template<typename K>
            inline static constexpr K get(const K& k) noexcept { return k; }
        };

        template<typename Storable, typename KeyExtractor, std::size_t TableSize>
        class PerfectHasher : public PerfectHasherStructurer<Storable, TableSize, Meta::is_structural<Storable>> {
        private:
            using Super = PerfectHasherStructurer<Storable, TableSize, Meta::is_structural<Storable>>;

            class Bucket {
            public:
                std::array<std::size_t, TableSize> members{};
                std::size_t count = 0;
            };

            inline static constexpr Super::Storage initializeStorage(std::array<Storable, TableSize>&& keys) noexcept {

            }

            inline static constexpr Super::Storage initializeStorage(std::array<constant_wrapper<Storable>, TableSize>&& keys) noexcept {

            }

            template<typename T>
            inline static consteval bool keysTypeCheck() noexcept {
                if constexpr (!std::same_as<Meta::toUnderlyingType<T>(), Meta::toUnderlyingType<std::array>()>)
                    return false;

                constexpr auto info = Meta::getTemplateInfo<T>();
                return std::same_as<typename decltype(info.templates.front())::type_holder::type, Storable> && are_values_equal<info.templates.back().value(), TableSize>();
            }

            inline constexpr PerfectHasher(Super::Storage&& storage) noexcept : Super(std::move(storage)) {}
        protected:
            inline constexpr PerfectHasher(std::array<Storable, TableSize> keys) noexcept : PerfectHasher(initializeStorage(std::move(keys))) {}
            inline constexpr PerfectHasher(auto keys) noexcept requires(keysTypeCheck<decltype(keys)>()) : PerfectHasher(initializeStorage(std::move(keys))) {}
        };
    }
}

#endif