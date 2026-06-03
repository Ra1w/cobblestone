#include "core/types/id.hpp"

#include <gtest/gtest.h>

#include "core/exceptions.hpp"

namespace {

TEST(TypesTest, IdGenerationIsUnique) {
  core::ID id1 = core::ID::Generate();
  core::ID id2 = core::ID::Generate();
  EXPECT_NE(id1, id2);
  EXPECT_FALSE(id1.Str().empty());
}

TEST(TypesTest, IdEqualityAndSorting) {
  core::ID id1("abc");
  core::ID id2("abc");
  core::ID id3("xyz");

  EXPECT_EQ(id1, id2);
  EXPECT_NE(id1, id3);
  EXPECT_LT(id1, id3);
}

}  // namespace
