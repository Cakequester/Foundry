#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

namespace list {
	constexpr auto marker = Foundry::FixedString("Marker");
	using list = Foundry::ValueList<0, Foundry::FixedString("Hello, World!"), 32, 54.43, marker>;

	constexpr int size = 5;
}

TEST(ValueList, At) {
	EXPECT_TRUE((Foundry::are_values_equal<list::list::back(), list::marker>()));
}

TEST(ValueList, Contains) {
	EXPECT_TRUE(list::list::contains<list::marker>());
}

TEST(ValueList, IndexOf) {
	EXPECT_EQ(list::list::indexOf<list::marker>(), list::list::size() - 1);
}

TEST(ValueList, Reverse) {
	using reversed = decltype(list::list::reverse());
	EXPECT_TRUE((Foundry::are_values_equal<reversed::front(), list::marker>()));
}

TEST(ValueList, Sublist) {
	using sublist = decltype(list::list::sublist<list::list::size() - 1, list::list::size()>());
	EXPECT_TRUE((Foundry::are_values_equal<sublist::front(), list::marker>()));
}

TEST(ValueList, PushFront) {
	using pushed = decltype(list::list::pushFront<list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<pushed::front(), list::marker>()));
}

TEST(ValueList, Append) {
	using appended = decltype(list::list::append<list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(ValueList, AppendOther) {
	using appended = decltype(list::list::appendList<Foundry::ValueList<list::marker, list::marker>>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(ValueList, AppendRange) {
	using appended = decltype(list::list::append<list::marker, list::marker>());
	EXPECT_TRUE((Foundry::are_values_equal<appended::back(), list::marker>()));
}

TEST(ValueList, Remove) {
	using removed = decltype(list::list::remove<list::marker>());
	EXPECT_FALSE(removed::contains<list::marker>());
}

TEST(ValueList, Size) {
	EXPECT_EQ(list::list::size(), list::size);
}

TEST(ValueList, Empty) {
	EXPECT_FALSE(list::list::empty());
}