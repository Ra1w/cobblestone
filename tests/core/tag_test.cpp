#include "core/types/tag.hpp"

#include <gtest/gtest.h>

#include <string>

#include "core/exceptions.hpp"

namespace {

class InvalidTagTest : public ::testing::TestWithParam<std::string> {};

TEST_P(InvalidTagTest, ThrowsValidationError) {
  EXPECT_THROW(
      {
        auto t = core::Tag(GetParam());
        (void)t;
      },
      core::ValidationError);
}

INSTANTIATE_TEST_SUITE_P(TagEdgeCases, InvalidTagTest,
                         ::testing::Values("", "tag,with,comma", "tag[bracket]",
                                           "tag\nnewline", "tag\rcarriage"));

TEST(TypesTest, TagNormalization) {
  core::Tag t("  UpPeR-CaSe  ");
  EXPECT_EQ(t.Str(), "upper-case");
}

}  // namespace
