#ifndef FOUNDRY_PERFECT_HASHING_H_
#define FOUNDRY_PERFECT_HASHING_H_

#include <cstddef>
#include <cstdint>

#include <array>
#include <tuple>

#include <optional>

#include <type_traits>
#include <concepts>

#include <utility>
#include <algorithm>
#include <numeric>

#include <foundry/meta/type_info.hpp>

#include <foundry/hashing.hpp>
#include <foundry/random.hpp>

namespace foundry {
    namespace detail {
        template<typename Storable, std::size_t TableSize>
        class perfect_hasher_storage {
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
        class perfect_hasher_structurer {
        public:
            using storage = perfect_hasher_storage<Storable, TableSize>;
        protected:
            const storage _storage;

            constexpr perfect_hasher_structurer(storage storage) noexcept : _storage(std::move(storage)) {}
        };

        template<typename Storable, std::size_t TableSize>
        class perfect_hasher_structurer<Storable, TableSize, true> {
        public:
            using storage = perfect_hasher_storage<Storable, TableSize>;
            const storage _storage;
        protected:
            constexpr perfect_hasher_structurer(storage storage) noexcept : _storage(std::move(storage)) {}
        };

        class key_extractor {};

        class pair_extractor : public key_extractor {
        public:
            template<typename P>
                requires(meta::instance_of<P, meta::to_underlying_type<std::pair>()>())
            static constexpr auto get(const P& p) noexcept { return p.first; }
        };

        class identity_extractor : public key_extractor {
        public:
            template<typename K>
            static constexpr K get(const K& k) noexcept { return k; }
        };

        template<typename Key, typename Storable, typename KeyExtractor, std::size_t TableSize>
            requires(std::derived_from<KeyExtractor, key_extractor>)
        class perfect_hasher : public perfect_hasher_structurer<Storable, TableSize, meta::is_structural<Storable>> {
        private:
            inline static constexpr std::uint64_t TopLevelSeed = mixed_type_identifier_random<perfect_hasher>::get() ^ mix_bits(TableSize);
            inline static constexpr std::uint64_t SecondarySeed = mixed_type_identifier_random<perfect_hasher>::get() ^ mix_bits(TableSize ^ mixed_type_identifier_random<perfect_hasher>::get());

            using super = perfect_hasher_structurer<Storable, TableSize, meta::is_structural<Storable>>;

            inline static constexpr std::size_t MaximumItemsForConsteval = 150;

            class Bucket {
            public:
                std::array<std::size_t, TableSize> members{};
                std::size_t count = 0;
            };

            static constexpr bool contains_duplicate_keys(const std::array<Storable, TableSize>& entries) noexcept {
                for (std::size_t i = 0; i < entries.size(); ++i) {
                    for (std::size_t j = i + 1; j < entries.size(); ++j) {
                        if (KeyExtractor::get(entries[i]) == KeyExtractor::get(entries[j]))
                            return true;
                    }
                }
                return false;
            }

            static constexpr super::storage initialize_storage(std::array<Storable, TableSize> entries) {
                if consteval {
                    if (entries.size() > MaximumItemsForConsteval)
                        // TODO: Figure out if this is the most idiomatic way to do this.
                        throw std::logic_error(std::string("Perfect hash types constructed at compile time cannot exceed ").append(std::to_string(MaximumItemsForConsteval)).append(" items!"));
                }

                if (entries.size() > TableSize)
                    throw std::logic_error("A perfect hash type cannot take more entries than it can hold!");

                if (contains_duplicate_keys(entries))
                    throw std::logic_error("A perfect hash type cannot have duplicate keys!");

                typename super::storage storage;
                using storage_type = decltype(storage);

                std::array<Bucket, storage_type::BucketCount> buckets = {};

                for (std::size_t i = 0; i < entries.size(); ++i) {
                    const std::size_t bucket = fnv1a::hash(KeyExtractor::get(entries[i]), TopLevelSeed) % storage_type::BucketCount;

                    buckets[bucket].members[buckets[bucket].count++] = i;
                }

                std::array<std::size_t, storage_type::BucketCount> order{};
                std::iota(order.begin(), order.end(), 0);

                std::sort(order.begin(), order.end(), [&buckets](std::size_t a, std::size_t b) {
                    return buckets[a].count > buckets[b].count;
                });

                std::array<bool, TableSize> occupied{};
                storage._table.fill(TableSize);

                for (std::size_t i = 0; i < storage_type::BucketCount; ++i) {
                    const std::size_t bucket_index = order[i];
                    const Bucket& bucket = buckets[bucket_index];

                    std::size_t displacement = 0;
                    while (true) {
                        std::array<std::size_t, TableSize> slots{};
                        bool valid = true;

                        for (std::size_t j = 0; j < bucket.count; ++j) {
                            const std::size_t entry_index = bucket.members[j];

                            const auto& entry = entries[entry_index];

                            const std::size_t slot = fnv1a::hash(KeyExtractor::get(entry), SecondarySeed ^ displacement) % TableSize;

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
                            storage._displacements[bucket_index] = displacement;

                            for (std::size_t j = 0; j < bucket.count; ++j) {
                                occupied[slots[j]] = true;
                                storage._table[slots[j]] = bucket.members[j];
                            }

                            break;
                        }

                        ++displacement;
                    }
                }

                storage._entries = std::move(entries);

                return storage;
            }

            constexpr perfect_hasher(super::storage storage) noexcept : super(std::move(storage)) {}
        protected:
            constexpr perfect_hasher(std::array<Storable, TableSize> entries) : perfect_hasher(std::move(initialize_storage(entries))) {}

            constexpr std::optional<Storable> get_data(Key key) const noexcept {
                const std::size_t bucket = fnv1a::hash(key, TopLevelSeed) % super::storage::BucketCount;

                const std::size_t displacement = this->_storage._displacements[bucket];

                const std::size_t slot = fnv1a::hash(key, SecondarySeed ^ displacement) % TableSize;

                const std::size_t entry_index = this->_storage._table[slot];

                if (entry_index == TableSize)
                    return std::nullopt;

                if (KeyExtractor::get(this->_storage._entries[entry_index]) != key)
                    return std::nullopt;

                return this->_storage._entries[entry_index];
            }
        public:
            inline constexpr std::size_t size() const noexcept {
                return TableSize;
            }
        };
    }

    template<typename Key, typename Value, std::size_t TableSize>
    class perfect_map : public detail::perfect_hasher<Key, std::pair<Key, Value>, detail::pair_extractor, TableSize> {
    private:
        using super = detail::perfect_hasher<Key, std::pair<Key, Value>, detail::pair_extractor, TableSize>;
    public:
        constexpr perfect_map(std::array<std::pair<Key, Value>, TableSize> entries) : super(entries) {}

        constexpr std::optional<Value> get(Key key) const noexcept {
            auto data = super::get_data(key);
            if (!data) return {};
            return (*data).second;
        }

        constexpr Value at(Key key) const {
            auto v = get(key);
            if (!v) throw std::out_of_range("Key not found!");
            return *v;
        }

        constexpr bool contains(Key key) const noexcept {
            auto data = super::get_data(key);
            return data.has_value();
        }
    };

    template<typename Key, typename Value, typename... Args>
        requires((std::same_as<Args, std::pair<Key, Value>> && ...))
    inline constexpr perfect_map<Key, Value, sizeof...(Args)> make_perfect_map(Args&&... args) {
        return perfect_map<Key, Value, sizeof...(Args)>(
            std::array{ std::forward<Args>(args)... }
        );
    }

    template<typename Key, std::size_t TableSize>
    class perfect_set : public detail::perfect_hasher<Key, Key, detail::identity_extractor, TableSize> {
    private:
        using super = detail::perfect_hasher<Key, Key, detail::identity_extractor, TableSize>;
    public:
        constexpr perfect_set(std::array<Key, TableSize> entries) : super(entries) {}

        constexpr bool contains(Key key) const noexcept {
            auto data = super::get_data(key);
            return data.has_value();
        }
    };

    template<typename Key, typename... Args>
        requires(((std::same_as<Args, Key> || std::convertible_to<Args, Key>) && ...))
    inline constexpr perfect_set<Key, sizeof...(Args)> make_perfect_set(Args&&... args) {
        return perfect_set<Key, sizeof...(Args)>(
            std::array{ Key(std::forward<Args>(args))... }
        );
    }
}

#endif