#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

namespace list {
	constexpr auto marker = Foundry::FixedString("Marker");
	using list = Foundry::ValueList<0, Foundry::FixedString("Hello, World!"), 32, 54.43, marker>;

	constexpr int size = 5;
}

TEST(TypeList, At) {
	EXPECT_TRUE((Foundry::are_values_equal<list::list::back(), list::marker>()));
}

TEST(TypeList, Contains) {
	EXPECT_TRUE(list::list::contains<list::marker>());
}

TEST(TypeList, IndexOf) {
	EXPECT_EQ(list::list::indexOf<list::marker>(), list::list::size() - 1);
}

TEST(TypeList, Reverse) {
	using reversed = decltype(list::list::reverse());
	EXPECT_TRUE((Foundry::are_values_equal<reversed::front(), list::marker>()));
}

TEST(TypeList, Sublist) {
	using sublist = decltype(list::list::sublist<list::list::size() - 1, list::list::size()>());
	EXPECT_TRUE((Foundry::are_values_equal<sublist::front(), list::marker>()));
}

TEST(TypeList, PushFront) {
	using pushed = decltype(list::list::pushFront<list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<pushed::front(), list::marker>()));
}

TEST(TypeList, Append) {
	using appended = decltype(list::list::append<list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(TypeList, AppendOther) {
	using appended = decltype(list::list::appendList<Foundry::ValueList<list::marker, list::marker>>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(TypeList, AppendRange) {
	using appended = decltype(list::list::appendRange<list::marker, list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(TypeList, Remove) {
	using removed = decltype(list::list::remove<list::marker>());
	EXPECT_FALSE(removed::contains<list::marker>());
}

TEST(TypeList, Size) {
	EXPECT_EQ(list::list::size(), list::size);
}

TEST(TypeList, Empty) {
	EXPECT_FALSE(list::list::empty());
}