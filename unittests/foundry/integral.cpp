#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

TEST(BitInt, EqualsUnsignedIntegral) {
	constexpr Foundry::uint128_t x = 83;
	EXPECT_EQ(x, 83);
}

TEST(BitInt, EqualsUnsignedOther) {
	constexpr Foundry::uint128_t x = 451;
	constexpr Foundry::uint256_t y = 451;
	EXPECT_EQ(x, y);
}

TEST(BitInt, EqualsIntegral) {
	constexpr Foundry::int128_t x = 651;
	EXPECT_EQ(x, 651);
}

TEST(BitInt, EqualsOther) {
	constexpr Foundry::int128_t x = 683;
	constexpr Foundry::int256_t y = 683;
	EXPECT_EQ(x, y);
}

TEST(BitInt, EqualsMismatchedOther) {
	constexpr Foundry::int128_t x = 704;
	constexpr Foundry::uint256_t y = 704;
	EXPECT_EQ(x, y);
}

TEST(BitInt, LessThanUnsignedIntegral) {
	constexpr Foundry::uint128_t x = 21;
	EXPECT_LT(x, 22);
}

TEST(BitInt, LessThanUnsignedOther) {
	constexpr Foundry::uint128_t x = 451;
	constexpr Foundry::uint256_t y = 456;
	EXPECT_LT(x, y);
}

TEST(BitInt, LessThanIntegral) {
	constexpr Foundry::int128_t x = 651;
	EXPECT_LT(x, 7012);
}

TEST(BitInt, LessThanOther) {
	constexpr Foundry::int128_t x = 683;
	constexpr Foundry::int256_t y = 95123;
	EXPECT_LT(x, y);
}

TEST(BitInt, LessThanMismatchedOther) {
	constexpr Foundry::int128_t x = -390;
	constexpr Foundry::uint256_t y = 6902;
	EXPECT_LT(x, y);
}

TEST(BitInt, GreaterThanUnsignedIntegral) {
	constexpr Foundry::uint128_t x = 21;
	EXPECT_GT(x, -19);
}

TEST(BitInt, GreaterThanUnsignedOther) {
	constexpr Foundry::uint128_t x = 401;
	constexpr Foundry::uint256_t y = 100;
	EXPECT_GT(x, y);
}

TEST(BitInt, GreaterThanIntegral) {
	constexpr Foundry::int128_t x = 7012;
	EXPECT_GT(x, 651);
}

TEST(BitInt, GreaterThanOther) {
	constexpr Foundry::int128_t x = 95123;
	constexpr Foundry::int256_t y = -59983;
	EXPECT_GT(x, y);
}

TEST(BitInt, GreaterThanMismatchedOther) {
	constexpr Foundry::int128_t x = 6902;
	constexpr Foundry::uint256_t y = 604;
	EXPECT_GT(x, y);
}