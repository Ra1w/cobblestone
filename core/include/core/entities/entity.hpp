#pragma once

#include <generator>
#include <memory>
#include <string>
#include <vector>

#include "core/types/base.hpp"
#include "core/types/id.hpp"
#include "core/types/tag.hpp"
#include "core/types/timestamp.hpp"
#include "core/types/title.hpp"

namespace core::entities {

struct Metadata {
  ID id;
  Title title;
  TagList tags;
  Timestamp created_at;
  Timestamp updated_at;

  Metadata(ID id, Title title, TagList tags);
};

class Entity {
 public:
  Entity(Metadata meta, std::string content = "");
  virtual ~Entity() = default;

  Entity(const Entity&) = delete;
  Entity& operator=(const Entity&) = delete;

  Entity(Entity&&) noexcept = default;
  Entity& operator=(Entity&&) noexcept = default;

  const ID& GetId() const { return meta_.id; }
  const Metadata& GetMetadata() const { return meta_; }
  const std::string& GetContent() const { return content_; }
  Entity* GetParent() const { return parent_; }
  const std::vector<std::unique_ptr<Entity>>& GetChildren() const {
    return children_;
  }

  void SetContent(std::string content);
  void SetTitle(Title title);
  void SetTags(TagList tags);

  virtual void RestoreStateFrom(const Entity& other);
  virtual std::unique_ptr<Entity> CloneWithoutChildren() const = 0;

  void AddChild(std::unique_ptr<Entity> child);
  std::unique_ptr<Entity> RemoveChild(const ID& id);

  virtual EntityType GetType() const = 0;
  virtual std::unique_ptr<Entity> Clone() const = 0;

  std::generator<Entity*> WalkTree();

 protected:
  Metadata meta_;
  std::string content_;

  Entity* parent_ = nullptr;
  std::vector<std::unique_ptr<Entity>> children_;

  void UpdateTimestamp();
};

}  // namespace core::entities
