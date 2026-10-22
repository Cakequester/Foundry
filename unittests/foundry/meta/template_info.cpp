#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

template<typename T, int A>
class Foo {};

TEST(TemplateInfo, Types) {
	constexpr auto Info = Foundry::Meta::getTemplateInfo<Foo<float, 30>>();
	EXPECT_TRUE(Info.Types.contains<float>());
}