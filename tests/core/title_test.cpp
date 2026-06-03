#include "core/types/title.hpp"

#include <gtest/gtest.h>

#include <string>

#include "core/exceptions.hpp"

namespace {

class InvalidTitleTest : public ::testing::TestWithParam<std::string> {};

TEST_P(InvalidTitleTest, ThrowsValidationError) {
  EXPECT_THROW(
      {
        auto t = core::Title(GetParam());
        (void)t;
      },
      core::ValidationError);
}

INSTANTIATE_TEST_SUITE_P(TitleEdgeCases, InvalidTitleTest,
                         ::testing::Values("", "   ", "\t\t", "Line1\nLine2",
                                           "Line1\rLine2",
                                           std::string(300, 'A')));

TEST(TypesTest, TitleTrimsWhitespaces) {
  core::Title t("  Hello World  \t");
  EXPECT_EQ(t.Str(), "Hello World");
}

}  // namespace
