#include "core/logic/registry.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "core/entities/note.hpp"
#include "core/exceptions.hpp"
#include "core/types/id.hpp"
#include "core/types/title.hpp"

namespace {

class RegistryTest : public ::testing::Test {
 protected:
  core::logic::Registry<core::entities::Entity> registry;
};

TEST_F(RegistryTest, ResolveIdAmbiguity) {
  auto note1 = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("abcd-1111"), core::Title("1"), {}));
  auto note2 = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("abcd-2222"), core::Title("2"), {}));

  registry.Add(std::move(note1));
  registry.Add(std::move(note2));

  EXPECT_THROW({ registry.ResolveId("abcd"); }, core::CommandError);

  EXPECT_NO_THROW({ registry.ResolveId("abcd-1111"); });

  EXPECT_THROW({ registry.ResolveId("ffff"); }, core::NotFoundError);
}

TEST_F(RegistryTest, PreventCircularDependency) {
  auto parent = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("parent"), core::Title("P"), {}));
  auto child = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("child"), core::Title("C"), {}));

  registry.Add(std::move(parent));
  registry.Add(std::move(child));

  registry.MoveEntity(core::ID("child"), core::ID("parent"));
  EXPECT_EQ(registry.Get(core::ID("child"))->GetParent()->GetId().Str(),
            "parent");

  EXPECT_THROW(
      { registry.MoveEntity(core::ID("parent"), core::ID("child")); },
      core::CommandError);
}

TEST_F(RegistryTest, BasicCrudOperations) {
  auto note = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("test-1"), core::Title("Title"), {}));

  EXPECT_NO_THROW({ registry.Add(std::move(note)); });
  EXPECT_EQ(registry.Count(), 1);

  auto* retrieved = registry.Get(core::ID("test-1"));
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->GetMetadata().title.Str(), "Title");

  EXPECT_EQ(registry.Get(core::ID("missing")), nullptr);

  std::unique_ptr<core::entities::Entity> removed;
  EXPECT_NO_THROW({ removed = registry.Remove(core::ID("test-1")); });
  EXPECT_NE(removed, nullptr);
  EXPECT_EQ(registry.Count(), 0);

  EXPECT_THROW({ registry.Remove(core::ID("test-1")); }, core::NotFoundError);
}

TEST_F(RegistryTest, FindIfAndHierarchy) {
  auto parent = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("p1"), core::Title("Parent"), {}));
  auto child = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID("c1"), core::Title("Child"), {}));

  registry.Add(std::move(parent));
  registry.Add(std::move(child));
  registry.MoveEntity(core::ID("c1"), core::ID("p1"));

  auto results = registry.FindIf([](const core::entities::Entity& e) {
    return e.GetMetadata().title.Str() == "Child";
  });

  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0]->GetParent()->GetId().Str(), "p1");
}

}  // namespace
