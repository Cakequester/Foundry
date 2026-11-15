#include <gtest/gtest.h>

#include <foundry/foundry.hpp>

template<typename T, int A>
class Foo {};

TEST(TemplateInfo, Types) {
	constexpr auto info = foundry::meta::get_template_info<Foo<float, 30>>();
	EXPECT_TRUE(info.templates.contains<float>());
}