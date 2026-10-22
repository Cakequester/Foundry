#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

namespace list {
	using marker_t = long double;
	using list = Foundry::TypeList<int, float, int, int, marker_t>;

	constexpr int size = 5;
}

TEST(TypeList, At) {
	EXPECT_TRUE((std::same_as<decltype(list::list::back())::type, list::marker_t>));
}

TEST(TypeList, Contains) {
	EXPECT_TRUE(list::list::contains<list::marker_t>());
}

TEST(TypeList, IndexOf) {
	EXPECT_EQ(list::list::indexOf<list::marker_t>(), list::list::size() - 1);
}

TEST(TypeList, Reverse) {
	using reversed = decltype(list::list::reverse());
	EXPECT_TRUE((std::same_as<decltype(reversed::front())::type, list::marker_t>));
}

TEST(TypeList, Sublist) {
	using sublist = decltype(list::list::sublist<list::list::size() - 1, list::list::size()>());
	EXPECT_TRUE((std::same_as<decltype(sublist::front())::type, list::marker_t>));
}

TEST(TypeList, PushFront) {
	using pushed = decltype(list::list::pushFront<list::marker_t>());
	EXPECT_TRUE((std::same_as<decltype(pushed::front())::type, list::marker_t>));
}

TEST(TypeList, Append) {
	using appended = decltype(list::list::append<list::marker_t>());
	EXPECT_TRUE((std::same_as<decltype(appended::back())::type, list::marker_t>));
}

TEST(TypeList, AppendOther) {
	using appended = decltype(list::list::appendList<Foundry::TypeList<list::marker_t, list::marker_t>>());
	EXPECT_TRUE((std::same_as<decltype(appended::back())::type, list::marker_t>));
}

TEST(TypeList, AppendRange) {
	using appended = decltype(list::list::append<list::marker_t, list::marker_t>());
	EXPECT_TRUE((std::same_as<decltype(appended::back())::type, list::marker_t>));
}

TEST(TypeList, Remove) {
	using removed = decltype(list::list::remove<list::marker_t>());
	EXPECT_FALSE(removed::contains<list::marker_t>());
}

TEST(TypeList, Size) {
	EXPECT_EQ(list::list::size(), list::size);
}

TEST(TypeList, Empty) {
	EXPECT_FALSE(list::list::empty());
}