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

namespace Foundry {
    template<std::size_t Size, bool Unsigned = false>
    class BitInt {
        static_assert(Size > 0, "BitInt must be greater than zero!");
    private:
        inline static constexpr std::size_t SizeInBytes = (Size + std::numeric_limits<std::uint8_t>::digits - 1) / std::numeric_limits<std::uint8_t>::digits;

        template<typename T>
        struct potentially_floating_to_integral {
            using type = std::make_unsigned_t<std::conditional_t<std::floating_point<T>, matching_integral_t<T>, T>>;
        };

        template<typename T>
        inline static constexpr std::size_t getMSB() noexcept {
            if constexpr (std::endian::native == std::endian::little) return sizeof(T) - 1;
            return 0;
        }

        template<std::size_t N>
        inline static constexpr std::size_t getMSB() noexcept {
            if constexpr (std::endian::native == std::endian::little) return N - 1;
            return 0;
        }

        inline constexpr void normalize() noexcept {
            constexpr unsigned extra = SizeInBytes * 8 - Size;
            if constexpr (extra != 0) {
                std::uint8_t& top = _data[getMSB<SizeInBytes>()];
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
            requires(instance_of<T, Foundry::BitInt>())
        inline constexpr void assign(const T& value) noexcept {
            _data.fill(isNegative(value) ? 0xFF : 0x00);
            for (std::size_t i = 0; i < std::min(sizeof(value), SizeInBytes); ++i)
                _data[i] = value._data[i];

            normalize();
        }

        inline constexpr bool isNegative() const noexcept {
            if constexpr (Unsigned) return false;

            if constexpr (std::endian::native == std::endian::little)
                return (_data[SizeInBytes - 1] & 0x80) != 0;
            else
                return (_data[0] & 0x80) != 0;
        }

        template<typename T>
            requires(instance_of<T, Foundry::BitInt>())
        inline static constexpr bool isNegative(const T& v) noexcept {
            if constexpr (T::IsUnsigned) return false;
            else return (v._data[getMSB<T>()] & 0x80) != 0;
        }
    public:
        inline static constexpr bool IsUnsigned = Unsigned;

        std::array<std::uint8_t, SizeInBytes> _data{};

        // ASSIGNMENT

        inline constexpr BitInt() {}

        template<typename T>
            requires(std::is_arithmetic_v<std::remove_cvref_t<T>>)
        inline constexpr BitInt(T value) noexcept {
            assign(value);
        }

        template<typename T>
            requires(instance_of<T, Foundry::BitInt>())
        inline constexpr BitInt(T value) noexcept {
            assign(value);
        }

        template<typename T>
            requires(std::is_arithmetic_v<std::remove_cvref_t<T>>)
        inline constexpr BitInt& operator=(T rhs) noexcept {
            assign(rhs);
            return *this;
        }

        template<typename T>
            requires(instance_of<T, Foundry::BitInt>())
        inline constexpr BitInt& operator=(T rhs) noexcept {
            assign(rhs);
            return *this;
        }

        // EQUALITY

        template<std::integral T>
        inline constexpr bool operator==(T rhs) const noexcept {
            const bool lhs_negative = !Unsigned && isNegative();
            const bool rhs_negative = std::is_signed_v<T> && rhs < 0;

            if constexpr (!Unsigned || std::is_signed_v<T>) {
                if (lhs_negative != rhs_negative)
                    return false;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            BitInt<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[getMSB<decltype(lhs)>()] &= 0x7F;

            BitInt<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (std::is_signed_v<T>)
                new_rhs._data[getMSB<decltype(new_rhs)>()] &= 0x7F;

            return std::equal(lhs._data.begin(), lhs._data.end(), new_rhs._data.begin(), new_rhs._data.end());
        }

        template<std::integral T>
        inline constexpr std::strong_ordering operator<=>(T rhs) const noexcept {
            const bool lhs_negative = !Unsigned && isNegative();
            const bool rhs_negative = std::is_signed_v<T> && rhs < 0;

            if constexpr (!Unsigned || std::is_signed_v<T>) {
                if (lhs_negative != rhs_negative)
                    return lhs_negative ? std::strong_ordering::less : std::strong_ordering::greater;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            BitInt<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[getMSB<decltype(lhs)>()] &= 0x7F;

            BitInt<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (std::is_signed_v<T>)
                new_rhs._data[getMSB<decltype(new_rhs)>()] &= 0x7F;

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
            requires(instance_of<T, Foundry::BitInt>())
        inline constexpr bool operator==(const T& rhs) const noexcept {
            const bool lhs_negative = !Unsigned && isNegative();
            const bool rhs_negative = !T::IsUnsigned && isNegative(rhs);

            if constexpr (!Unsigned || !T::IsUnsigned) {
                if (lhs_negative != rhs_negative)
                    return false;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            BitInt<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[getMSB<decltype(lhs)>()] &= 0x7F;

            BitInt<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (!T::IsUnsigned)
                new_rhs._data[getMSB<decltype(new_rhs)>()] &= 0x7F;

            return std::equal(lhs._data.begin(), lhs._data.end(), new_rhs._data.begin(), new_rhs._data.end());
        }

        template<typename T>
            requires(instance_of<T, Foundry::BitInt>())
        inline constexpr std::strong_ordering operator<=>(const T& rhs) const noexcept {
            const bool lhs_negative = !Unsigned && isNegative();
            const bool rhs_negative = !T::IsUnsigned && isNegative(rhs);

            if constexpr (!Unsigned || !T::IsUnsigned) {
                if (lhs_negative != rhs_negative)
                    return lhs_negative ? std::strong_ordering::less : std::strong_ordering::greater;
            }

            constexpr std::size_t UniformSize = std::max(SizeInBytes, sizeof(rhs));

            BitInt<UniformSize * 8, true> lhs(*this);

            if constexpr (!Unsigned)
                lhs._data[getMSB<decltype(lhs)>()] &= 0x7F;

            BitInt<UniformSize * 8, true> new_rhs(rhs);

            if constexpr (!T::IsUnsigned)
                new_rhs._data[getMSB<decltype(new_rhs)>()] &= 0x7F;

            if constexpr (std::endian::native == std::endian::little)
                return std::lexicographical_compare_three_way(
                    lhs._data.rbegin(), lhs._data.rend(),
                    new_rhs._data.rbegin(), new_rhs._data.rend());
            else
                return std::lexicographical_compare_three_way(
                    lhs._data.begin(), lhs._data.end(),
                    new_rhs._data.begin(), new_rhs._data.end());
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
            requires(instance_of<T, Foundry::BitInt>())
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
    using UBitInt = BitInt<Size, true>;

    using int128_t = BitInt<128, false>;
    using uint128_t = BitInt<128, true>;

    using int256_t = BitInt<256, false>;
    using uint256_t = BitInt<256, true>;
}

#endif