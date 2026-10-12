#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

namespace set {
	constexpr int testing_key = 64213;

	constexpr int size = 3;

	constexpr auto set = Foundry::makePerfectHashSet<int>(314, testing_key, 56723);
}

TEST(CompileSetTest, Contains) {
	EXPECT_TRUE(set::set.contains(set::testing_key));
}

TEST(CompileSetTest, Size) {
	EXPECT_EQ(set::set.size(), set::size);
}