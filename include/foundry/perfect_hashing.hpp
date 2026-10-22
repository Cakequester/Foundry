#ifndef FOUNDRY_PERFECT_HASHING_H_
#define FOUNDRY_PERFECT_HASHING_H_

#include <cstddef>
#include <cstdint>

#include <array>

#include <type_traits>
#include <concepts>

#include <utility>
#include <algorithm>
#include <numeric>

#include <foundry/meta/type_info.hpp>

#include <foundry/hashing.hpp>
#include <foundry/random.hpp>

namespace Foundry {
    class PerfectHashKeyStrategy {};

    class PairFirstKeyStrategy : public PerfectHashKeyStrategy {
    public:
        template<typename P>
            requires(Meta::instance_of<P, std::pair>())
        inline static constexpr auto get(const P& p) noexcept { return p.first; }
    };

    class IdentityKeyStrategy : public PerfectHashKeyStrategy {
    public:
        template<typename K>
        inline static constexpr K get(const K& k) noexcept { return k; }
    };

    template<typename T>
    concept perfect_hash_key_strategy = std::derived_from<T, PerfectHashKeyStrategy>;

    template<typename Key, typename T, typename KeyOf, std::size_t TableSize>
        requires(perfect_hash_key_strategy<KeyOf>)
    class PerfectHasher {
    private:
        inline static constexpr std::size_t BucketCount = TableSize == 0 ? 1 : TableSize;

        class Bucket {
        public:
            std::array<std::size_t, TableSize> members{};
            std::size_t count = 0;
        };

        class BuiltAggregate {
        public:
            std::array<std::size_t, TableSize> _table{};
            std::array<std::size_t, BucketCount> _displacements{};

            inline static constexpr bool checkForDuplicates(const std::array<T, TableSize>& entries) noexcept {
                for (std::size_t i = 0; i < entries.size(); ++i) {
                    for (std::size_t j = i + 1; j < entries.size(); ++j) {
                        if (KeyOf::get(entries[i]) == KeyOf::get(entries[j]))
                            return true;
                    }
                }
                return false;
            }

            inline constexpr BuiltAggregate(const std::array<T, TableSize>& entries) {
                if (checkForDuplicates(entries))
                    throw std::logic_error("A perfect hash type cannot have duplicate keys!");

                std::array<Bucket, BucketCount> buckets = {};

                for (std::size_t i = 0; i < entries.size(); ++i) {
                    const std::size_t bucket = FNV1A::hash(KeyOf::get(entries[i]), TopLevelSeed) % BucketCount;

                    buckets[bucket].members[buckets[bucket].count++] = i;
                }

                std::array<std::size_t, BucketCount> order{};
                std::iota(order.begin(), order.end(), 0);

                std::sort(order.begin(), order.end(), [&buckets](std::size_t a, std::size_t b) {
                    return buckets[a].count > buckets[b].count;
                });

                std::array<bool, TableSize> occupied{};
                _table.fill(TableSize);

                for (std::size_t i = 0; i < BucketCount; ++i) {
                    const std::size_t bucket_index = order[i];
                    const Bucket& bucket = buckets[bucket_index];

                    std::size_t displacement = 0;
                    while (true) {
                        std::array<std::size_t, TableSize> slots{};
                        bool valid = true;

                        for (std::size_t j = 0; j < bucket.count; ++j) {
                            const std::size_t entry_index = bucket.members[j];

                            const auto& entry = entries[entry_index];

                            const std::size_t slot = FNV1A::hash(KeyOf::get(entry), SecondarySeed ^ displacement) % TableSize;

                            if (occupied[slot]) {
                                valid = false;
                                break;
                            }

                            for (std::size_t k = 0; k < j; ++k) {
                                if (slots[k] == slot) {
                                    valid = false;
                                    break;
                                }
                            }

                            if (!valid) break;

                            slots[j] = slot;
                        }

                        if (valid) {
                            _displacements[bucket_index] = displacement;

                            for (std::size_t j = 0; j < bucket.count; ++j) {
                                occupied[slots[j]] = true;
                                _table[slots[j]] = bucket.members[j];
                            }

                            break;
                        }

                        ++displacement;
                    }
                }
            }
        };
    public:
        const std::array<T, TableSize> _entries;
        const std::array<std::size_t, TableSize> _table{};
        const std::array<std::size_t, BucketCount> _displacements{};
    private:
        inline constexpr PerfectHasher(std::array<T, TableSize> entries, BuiltAggregate&& aggregate) : _entries(entries), _table(std::move(aggregate._table)), _displacements(std::move(aggregate._displacements)) {}
    protected:
        inline static constexpr std::uint64_t TopLevelSeed = MixedTypeIdentifierRandom<PerfectHasher>::get() ^ mix_bits(TableSize);
        inline static constexpr std::uint64_t SecondarySeed = MixedTypeIdentifierRandom<PerfectHasher>::get() ^ mix_bits(TableSize ^ MixedTypeIdentifierRandom<PerfectHasher>::get());

        inline constexpr PerfectHasher(std::array<T, TableSize> entries) noexcept : PerfectHasher(entries, BuiltAggregate(entries)) {}

        inline constexpr std::optional<T> getData(Key key) const noexcept {
            const std::size_t bucket = FNV1A::hash(key, TopLevelSeed) % BucketCount;

            const std::size_t displacement = _displacements[bucket];

            const std::size_t slot = FNV1A::hash(key, SecondarySeed ^ displacement) % TableSize;

            const std::size_t entry_index = _table[slot];

            if (entry_index == TableSize)
                return std::nullopt;

            if (KeyOf::get(_entries[entry_index]) != key)
                return std::nullopt;

            return _entries[entry_index];
        }
    public:
        inline constexpr std::size_t size() const noexcept {
            return TableSize;
        }
    };

    template<typename Key, typename Value, std::size_t TableSize>
    class PerfectHashMap : public PerfectHasher<Key, std::pair<Key, Value>, PairFirstKeyStrategy, TableSize> {
    private:
        using SuperType = PerfectHasher<Key, std::pair<Key, Value>, PairFirstKeyStrategy, TableSize>;
    public:
        inline constexpr PerfectHashMap(std::array<std::pair<Key, Value>, TableSize> entries) noexcept : SuperType(entries) {}

        inline constexpr std::optional<Value> get(Key key) const noexcept {
            auto data = SuperType::getData(key);
            if (!data) return {};
            return (*data).second;
        }

        inline constexpr Value at(Key key) const {
            auto v = get(key);
            if (!v) throw std::out_of_range("CompileMap: key not found");
            return *v;
        }

        inline constexpr bool contains(Key key) const noexcept {
            return get(key).has_value();
        }
    };

    template<typename Key, typename Value, typename... Args>
        requires ((std::same_as<Args, std::pair<Key, Value>> || std::convertible_to<Args, std::pair<Key, Value>>) && ...)
    constexpr auto makePerfectHashMap(Args&&... args) noexcept {
        return PerfectHashMap<Key, Value, sizeof...(Args)>(
            std::array<std::pair<Key, Value>, sizeof...(Args)>{
            std::pair<Key, Value>(std::forward<Args>(args))...
        });
    }

    template<typename Key, std::size_t TableSize>
    class PerfectHashSet : public PerfectHasher<Key, Key, IdentityKeyStrategy, TableSize> {
    private:
        using SuperType = PerfectHasher<Key, Key, IdentityKeyStrategy, TableSize>;
    public:
        inline constexpr PerfectHashSet(std::array<Key, TableSize> entries) noexcept : SuperType(entries) {}

        inline constexpr bool contains(Key key) const noexcept {
            auto data = SuperType::getData(key);
            return data.has_value();
        }
    };

    template<typename Key, typename... Args>
        requires ((std::same_as<Args, Key> || std::convertible_to<Args, Key>) && ...)
    constexpr auto makePerfectHashSet(Args&&... args) noexcept {
        return PerfectHashSet<Key, sizeof...(Args)>(
            std::array<Key, sizeof...(Args)>{Key(std::forward<Args>(args))...}
        );
    }
}

#endif