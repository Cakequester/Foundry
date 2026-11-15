#ifndef FOUNDRY_INTEGRAL_H_
#define FOUNDRY_INTEGRAL_H_

#include <cstddef>
#include <cstdint>
#include <cmath>

#include <array>

#include <type_traits>
#include <concepts>

#include <algorithm>
#include <utility>
#include <compare>

#include <limits>
#include <bit>

#include <foundry/compile_time.hpp>

namespace foundry {
    template<std::size_t Size, bool Unsigned = false>
    class bit_int {
    private:
        static_assert(Size > 0, "bit_int must be greater than zero!");

        template<std::floating_point T>
        struct matching_integral {
            using type = std::conditional_t<std::same_as<T, float>, std::uint32_t, std::uint64_t>;
        };

        template<typename T>
        using matching_integral_t = matching_integral<T>::type;

        inline static constexpr std::size_t SizeInBytes = (Size + std::numeric_limits<std::uint8_t>::digits - 1) / std::numeric_limits<std::uint8_t>::digits;

        template<typename T>
        struct potentially_floating_to_integral {
            using type = std::make_unsigned_t<std::conditional_t<std::floating_point<T>, matching_integral_t<T>, T>>;
        };

        template<typename T>
        inline static constexpr std::size_t get_msb() noexcept {
            if constexpr (std::endian::native == std::endian::little) return sizeof(T) - 1;
            return 0;
        }

        template<std::size_t N>
        inline static constexpr std::size_t get_msb() noexcept {
            if constexpr (std::endian::native == std::endian::little) return N - 1;
            return 0;
        }

        inline constexpr void normalize() noexcept {
            constexpr unsigned extra = SizeInBytes * 8 - Size;
            if constexpr (extra != 0) {
                std::uint8_t& top = _data[get_msb<SizeInBytes>()];
                const std::uint8_t keep = static_cast<std::uint8_t>(0xFFu >> extra);
                if constexpr (!Unsigned) {
                    const std::uint8_t signBit = static_cast<std::uint8_t>(0x80u >> extra);
                    if (top & signBit) top |= static_cast<std::uint8_t>(~keep);
                    else top &= keep;
                }
                else top &= keep;
            }
        }

        template<std::integral T>
        constexpr void assign(T value) noexcept {
            const auto castValue = static_cast<std::make_unsigned_t<T>>(value);

            constexpr std::size_t bytes_to_copy = std::min(sizeof(value), SizeInBytes);
            const auto val_bytes = std::bit_cast<std::array<std::uint8_t, sizeof(castValue)>>(castValue);

            _data.fill(std::is_signed_v<std::remove_cvref_t<T>> && value < 0 ? 0xFF : 0x00);
            std::copy_n(val_bytes.begin(), bytes_to_copy, _data.begin());

            normalize();
        }

        template<std::floating_point T>
        constexpr void assign(T value) noexcept {
            if (value != value) { _data.fill(0); return; }
            if (std::isinf(value)) { _data.fill(0); return; }

            double m = static_cast<double>(value);
            const bool neg = m < 0;
            m = neg ? -std::floor(-m) : std::floor(m);
            if (neg) m = -m;

            std::array<std::uint8_t, SizeInBytes> r{};
            for (std::size_t k = 0; k < SizeInBytes; ++k) {
                const double q = std::floor(m / 256.0);
                r[k] = static_cast<std::uint8_t>(m - q * 256.0);
                m = q;
            }
            if (neg) {
                unsigned carry = 1;
                for (auto& b : r) {
                    const unsigned v = static_cast<std::uint8_t>(~b) + carry;
                    b = static_cast<std::uint8_t>(v);
                    carry = v >> 8;
                }
            }
            for (std::size_t k = 0; k < SizeInBytes; ++k)
                _data[std::endian::native == std::endian::little ? k : SizeInBytes - 1 - k] = r[k];
            normalize();
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr void assign(const T& value) noexcept {
            _data.fill(is_negative(value) ? 0xFF : 0x00);
            for (std::size_t i = 0; i < std::min(sizeof(value), SizeInBytes); ++i)
                _data[i] = value._data[i];

            normalize();
        }

        inline constexpr bool is_negative() const noexcept {
            if constexpr (Unsigned) return false;

            if constexpr (std::endian::native == std::endian::little)
                return (_data[SizeInBytes - 1] & 0x80) != 0;
            else
                return (_data[0] & 0x80) != 0;
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline static constexpr bool is_negative(const T& v) noexcept {
            if constexpr (T::IsUnsigned) return false;
            else return (v._data[get_msb<T>()] & 0x80) != 0;
        }

        inline static constexpr std::size_t byte_index(std::size_t i) noexcept {
            if constexpr (std::endian::native == std::endian::little)
                return i;
            else
                return SizeInBytes - 1 - i;
        }

        template<std::integral T>
        inline static constexpr bool is_unsigned_type() noexcept {
            return std::is_unsigned_v<T>;
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline static constexpr bool is_unsigned_type() noexcept {
            return T::IsUnsigned;
        }

        template<typename T, typename U>
        inline static constexpr auto promote_integral() noexcept {
            constexpr std::size_t UniformSize = std::max(sizeof(T), sizeof(U));

            using UniformType = bit_int<UniformSize * 8, is_unsigned_type<T>() || is_unsigned_type<U>()>;

            if constexpr (meta::instance_of<UniformType, meta::to_underlying_type<foundry::bit_int>(), std::numeric_limits<std::uint8_t>::digits>())
                return std::conditional_t<UniformType::IsUnsigned, std::type_identity<std::uint8_t>, std::type_identity<std::int8_t>>{};
            else if constexpr (meta::instance_of<UniformType, meta::to_underlying_type<foundry::bit_int>(), std::numeric_limits<std::uint16_t>::digits>())
                return std::conditional_t<UniformType::IsUnsigned, std::type_identity<std::uint16_t>, std::type_identity<std::int16_t>>{};
            else if constexpr (meta::instance_of<UniformType, meta::to_underlying_type<foundry::bit_int>(), std::numeric_limits<std::uint32_t>::digits>())
                return std::conditional_t<UniformType::IsUnsigned, std::type_identity<std::uint32_t>, std::type_identity<std::int32_t>>{};
            else if constexpr (meta::instance_of<UniformType, meta::to_underlying_type<foundry::bit_int>(), std::numeric_limits<std::uint64_t>::digits>())
                return std::conditional_t<UniformType::IsUnsigned, std::type_identity<std::uint64_t>, std::type_identity<std::int64_t>>{};
            else
                return std::type_identity<UniformType>{};
        }

        template<typename T, typename U>
        using promoted_integral = typename decltype(promote_integral<T, U>())::type;
    public:
        inline static constexpr bool IsUnsigned = Unsigned;

        std::array<std::uint8_t, SizeInBytes> _data{};

        // ASSIGNMENT

        inline constexpr bit_int() {}

        template<typename T>
            requires(std::is_arithmetic_v<std::remove_cvref_t<T>>)
        inline constexpr bit_int(T value) noexcept {
            assign(value);
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr bit_int(T value) noexcept {
            assign(value);
        }

        template<typename T>
            requires(std::is_arithmetic_v<std::remove_cvref_t<T>>)
        inline constexpr bit_int& operator=(T rhs) noexcept {
            assign(rhs);
            return *this;
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr bit_int& operator=(T rhs) noexcept {
            assign(rhs);
            return *this;
        }

        // EQUALITY

        template<std::integral T>
        inline constexpr bool operator==(T rhs) const noexcept {
            const bool lhs_negative = !Unsigned && is_negative();
            const bool rhs_negative = std::is_signed_v<T> && rhs < 0;

            if constexpr (!Unsigned || std::is_signed_v<T>) {
                if (lhs_negative != rhs_negative)
                    return false;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[get_msb<decltype(lhs)>()] &= 0x7F;

            bit_int<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (std::is_signed_v<T>)
                new_rhs._data[get_msb<decltype(new_rhs)>()] &= 0x7F;

            return std::equal(lhs._data.begin(), lhs._data.end(), new_rhs._data.begin(), new_rhs._data.end());
        }

        template<std::integral T>
        inline constexpr std::strong_ordering operator<=>(T rhs) const noexcept {
            const bool lhs_negative = !Unsigned && is_negative();
            const bool rhs_negative = std::is_signed_v<T> && rhs < 0;

            if constexpr (!Unsigned || std::is_signed_v<T>) {
                if (lhs_negative != rhs_negative)
                    return lhs_negative ? std::strong_ordering::less : std::strong_ordering::greater;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[get_msb<decltype(lhs)>()] &= 0x7F;

            bit_int<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (std::is_signed_v<T>)
                new_rhs._data[get_msb<decltype(new_rhs)>()] &= 0x7F;

            if constexpr (std::endian::native == std::endian::little)
                return std::lexicographical_compare_three_way(
                    lhs._data.rbegin(), lhs._data.rend(),
                    new_rhs._data.rbegin(), new_rhs._data.rend());
            else
                return std::lexicographical_compare_three_way(
                    lhs._data.begin(), lhs._data.end(),
                    new_rhs._data.begin(), new_rhs._data.end());
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr bool operator==(const T& rhs) const noexcept {
            const bool lhs_negative = !Unsigned && is_negative();
            const bool rhs_negative = !T::IsUnsigned && is_negative(rhs);

            if constexpr (!Unsigned || !T::IsUnsigned) {
                if (lhs_negative != rhs_negative)
                    return false;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[get_msb<decltype(lhs)>()] &= 0x7F;

            bit_int<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (!T::IsUnsigned)
                new_rhs._data[get_msb<decltype(new_rhs)>()] &= 0x7F;

            return std::equal(lhs._data.begin(), lhs._data.end(), new_rhs._data.begin(), new_rhs._data.end());
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr std::strong_ordering operator<=>(const T& rhs) const noexcept {
            const bool lhs_negative = !Unsigned && is_negative();
            const bool rhs_negative = !T::IsUnsigned && is_negative(rhs);

            if constexpr (!Unsigned || !T::IsUnsigned) {
                if (lhs_negative != rhs_negative)
                    return lhs_negative ? std::strong_ordering::less : std::strong_ordering::greater;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[get_msb<decltype(lhs)>()] &= 0x7F;

            bit_int<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (!T::IsUnsigned)
                new_rhs._data[get_msb<decltype(new_rhs)>()] &= 0x7F;

            if constexpr (std::endian::native == std::endian::little)
                return std::lexicographical_compare_three_way(
                    lhs._data.rbegin(), lhs._data.rend(),
                    new_rhs._data.rbegin(), new_rhs._data.rend());
            else
                return std::lexicographical_compare_three_way(
                    lhs._data.begin(), lhs._data.end(),
                    new_rhs._data.begin(), new_rhs._data.end());
        }

        // BIT SHIFT

        inline constexpr bit_int& operator<<=(int shift) {
            if (shift < 0) throw std::invalid_argument("Negative shift");

            if (shift >= Size) {
                _data.fill(0);
                return *this;
            }

            const auto src = _data;
            const std::size_t byte_shift = shift / 8;
            const unsigned bit_shift = static_cast<unsigned>(shift % 8);

            for (std::size_t i = SizeInBytes; i-- > 0;) {
                std::uint16_t value = 0;

                if (i >= byte_shift) {
                    value = static_cast<std::uint16_t>(src[byte_index(i - byte_shift)]) << bit_shift;

                    if (bit_shift != 0 && i > byte_shift)
                        value |= static_cast<std::uint16_t>(src[byte_index(i - byte_shift - 1)]) >> (8 - bit_shift);
                }

                _data[byte_index(i)] = static_cast<std::uint8_t>(value);
            }

            normalize();
            return *this;
        }

        inline constexpr bit_int& operator>>=(int shift) {
            if (shift < 0) throw std::invalid_argument("Negative shift");

            const bool negative = is_negative();

            if (static_cast<std::size_t>(shift) >= Size) {
                _data.fill(negative ? 0xFF : 0x00);
                normalize();
                return *this;
            }

            const auto src = _data;
            const std::size_t byte_shift = static_cast<std::size_t>(shift) / 8;
            const unsigned bit_shift = static_cast<unsigned>(shift % 8);
            const std::uint8_t fill = negative ? 0xFF : 0x00;

            for (std::size_t i = 0; i < SizeInBytes; ++i) {
                const std::size_t source = i + byte_shift;
                std::uint16_t value = 0;

                if (source < SizeInBytes) {
                    value = src[byte_index(source)];

                    if (bit_shift != 0) {
                        const std::uint8_t upper = source + 1 < SizeInBytes
                            ? src[byte_index(source + 1)]
                            : fill;

                        value >>= bit_shift;
                        value |= static_cast<std::uint16_t>(upper) << (8 - bit_shift);
                    }
                }
                else {
                    value = fill;
                }

                _data[byte_index(i)] = static_cast<std::uint8_t>(value);
            }

            normalize();
            return *this;
        }

        inline constexpr bit_int operator<<(int shift) const {
            bit_int result(*this);
            result <<= shift;
            return result;
        }

        inline constexpr bit_int operator>>(int shift) const {
            bit_int result(*this);
            result >>= shift;
            return result;
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr bit_int& operator|=(const T& rhs) noexcept {
            const std::uint8_t extension_byte = (!T::IsUnsigned && is_negative(rhs)) ? 0xFF : 0x00;

            for (std::size_t i = 0; i < SizeInBytes; ++i) {
                std::uint8_t const rhs_byte = (i < sizeof(rhs)) ? rhs._data[i] : extension_byte;

                this->_data[i] |= rhs_byte;
            }

            return *this;
        }

        template<std::integral T>
        inline constexpr bit_int& operator|=(T rhs) noexcept {
            *this |= bit_int<sizeof(rhs) * 8, std::is_unsigned_v<T>>(rhs);
            return *this;
        }

        template<std::integral T>
        inline constexpr promoted_integral<T, bit_int> operator|(T rhs) const noexcept {
            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, Unsigned> lhs(*this);
            bit_int<UniformSize * 8, std::is_unsigned_v<T>> new_rhs(rhs);

            lhs |= new_rhs;

            return lhs;
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr promoted_integral<T, bit_int> operator|(const T& rhs) const noexcept {
            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            bit_int<UniformSize * 8, Unsigned> lhs(*this);
            bit_int<UniformSize * 8, T::IsUnsigned> new_rhs(rhs);

            lhs |= new_rhs;

            return lhs;
        }

        // CASTS

        template<std::integral T>
        inline constexpr operator T() const noexcept {
            std::array<std::uint8_t, sizeof(T)> result{};

            constexpr std::size_t bytes_to_copy = std::min(sizeof(T), SizeInBytes);
            std::copy_n(_data.begin(), bytes_to_copy, result.begin());
            return std::bit_cast<T>(result);
        }

        template<typename T>
            requires(meta::instance_of<T, meta::to_underlying_type<foundry::bit_int>()>())
        inline constexpr operator T() const noexcept {
            return T(*this);
        }

        template<std::floating_point T>
        inline constexpr operator T() const noexcept {
            if constexpr (Unsigned) return static_cast<T>(static_cast<std::uintmax_t>(*this));
            else return static_cast<T>(static_cast<std::intmax_t>(*this));
        }
    };

    template<std::size_t Size>
    using UBitInt = bit_int<Size, true>;

    using int128_t = bit_int<128, false>;
    using uint128_t = bit_int<128, true>;

    using int256_t = bit_int<256, false>;
    using uint256_t = bit_int<256, true>;
}

#endif