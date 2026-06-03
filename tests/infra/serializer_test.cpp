#include <gtest/gtest.h>

#include "core/entities/note.hpp"
#include "core/entities/task.hpp"
#include "core/exceptions.hpp"
#include "infra/serialization/markdown_serializer.hpp"

namespace {

using namespace infra::serialization;

TEST(SerializerTest, DeserializeValidNote) {
  std::string valid_markdown =
      "---\n"
      "id: 12345\n"
      "type: note\n"
      "title: Test Note\n"
      "tags: [tag1, tag2]\n"
      "created_at: 2026-01-01T12:00:00Z\n"
      "updated_at: 2026-01-01T12:00:00Z\n"
      "parent: null\n"
      "---\n"
      "Body of the note.";

  auto entry = MarkdownSerializer::Deserialize(valid_markdown);

  EXPECT_EQ(entry.entity->GetId().Str(), "12345");
  EXPECT_EQ(entry.entity->GetMetadata().title.Str(), "Test Note");
  EXPECT_EQ(entry.entity->GetContent(), "Body of the note.");
  EXPECT_EQ(entry.entity->GetMetadata().tags.size(), 2);
  EXPECT_EQ(entry.entity->GetMetadata().tags[0].Str(), "tag1");
  EXPECT_FALSE(entry.parent_id.has_value());
}

TEST(SerializerTest, ThrowsOnMissingYamlHeader) {
  std::string bad_markdown = "Just some text without header";
  EXPECT_THROW(
      { MarkdownSerializer::Deserialize(bad_markdown); },
      core::PersistenceError);
}

TEST(SerializerTest, ThrowsOnUnclosedYamlHeader) {
  std::string bad_markdown =
      "---\n"
      "id: 123\n"
      "type: note\n"
      "Body with no closing header";
  EXPECT_THROW(
      { MarkdownSerializer::Deserialize(bad_markdown); },
      core::PersistenceError);
}

TEST(SerializerTest, ThrowsOnMissingMandatoryFields) {
  std::string no_id = "---\ntype: note\n---\n";
  EXPECT_THROW(
      { MarkdownSerializer::Deserialize(no_id); }, core::PersistenceError);

  std::string no_type = "---\nid: 123\n---\n";
  EXPECT_THROW(
      { MarkdownSerializer::Deserialize(no_type); }, core::PersistenceError);
}

TEST(SerializerTest, SerializeNote) {
  core::entities::Metadata meta(core::ID("123"), core::Title("Note Title"), {core::Tag("tag1")});
  meta.created_at = core::Timestamp::FromIsoString("2026-01-01T12:00:00Z");
  meta.updated_at = core::Timestamp::FromIsoString("2026-01-02T12:00:00Z");
  core::entities::Note note(meta, "Note content");

  std::string result = MarkdownSerializer::Serialize(note);

  EXPECT_NE(result.find("id: 123"), std::string::npos);
  EXPECT_NE(result.find("type: note"), std::string::npos);
  EXPECT_NE(result.find("tags: [tag1]"), std::string::npos);
  EXPECT_NE(result.find("Note content"), std::string::npos);
  EXPECT_EQ(result.find("status:"), std::string::npos);
}

TEST(SerializerTest, SerializeAndDeserializeTask) {
  core::entities::Metadata meta(core::ID("task-1"), core::Title("Task Title"), {});
  core::entities::Task original_task(meta, "Task content", core::TaskStatus::Done);

  std::string yaml = MarkdownSerializer::Serialize(original_task);
  
  EXPECT_NE(yaml.find("type: task"), std::string::npos);
  EXPECT_NE(yaml.find("status: done"), std::string::npos);

  auto entry = MarkdownSerializer::Deserialize(yaml);
  
  ASSERT_EQ(entry.entity->GetType(), core::EntityType::Task);
  auto* restored_task = static_cast<core::entities::Task*>(entry.entity.get());
  
  EXPECT_EQ(restored_task->GetStatus(), core::TaskStatus::Done);
}

}  // namespace
