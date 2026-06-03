#include "core/types/timestamp.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include "core/exceptions.hpp"

namespace {

TEST(TypesTest, TimestampIsoConversion) {
  core::Timestamp now = core::Timestamp::Now();
  std::string iso = now.ToIsoString();

  core::Timestamp parsed = core::Timestamp::FromIsoString(iso);
  EXPECT_EQ(now.ToIsoString(), parsed.ToIsoString());
}

TEST(TypesTest, InvalidTimestampThrows) {
  EXPECT_THROW(
      { core::Timestamp::FromIsoString("2023-13-99 Not ISO"); },
      core::ValidationError);
}

TEST(TypesTest, TimestampComparisonOperators) {
  core::Timestamp t1;
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  core::Timestamp t2;

  EXPECT_TRUE(t1 < t2);
  EXPECT_FALSE(t1 > t2);
  EXPECT_TRUE(t1 != t2);
  EXPECT_FALSE(t1 == t2);

  core::Timestamp t1_copy = t1;
  EXPECT_TRUE(t1 == t1_copy);
}

}  // namespace
