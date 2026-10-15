#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

namespace map {
	constexpr int testing_key = 0;
	constexpr int testing_value = 64213;

	constexpr int size = 3;

	constexpr auto map = Foundry::makePerfectHashMap<int, int>(std::pair{ 314, 523 }, std::pair{ testing_key, testing_value }, std::pair{ 56723, 90993 });
}

TEST(CompileMap, ContainsKey) {
	EXPECT_TRUE(map::map.contains(map::testing_key));
}

TEST(CompileMap, ContainsValue) {
	EXPECT_EQ(map::map.at(map::testing_key), map::testing_value);

	auto value = map::map.get(map::testing_key);

	EXPECT_TRUE(value.has_value());
	EXPECT_EQ(value.value(), map::testing_value);
}

TEST(CompileMap, Size) {
	EXPECT_EQ(map::map.size(), map::size);
}