#include "core/entities/entity.hpp"

#include <algorithm>

#include "core/exceptions.hpp"

namespace core::entities {

Metadata::Metadata(ID id, Title title, TagList tags)
    : id(std::move(id)),
      title(std::move(title)),
      tags(std::move(tags)),
      created_at(Timestamp::Now()),
      updated_at(created_at) {}

Entity::Entity(Metadata meta, std::string content)
    : meta_(std::move(meta)), content_(std::move(content)) {}

void Entity::SetContent(std::string content) {
  content_ = std::move(content);
  UpdateTimestamp();
}

void Entity::SetTitle(Title title) {
  meta_.title = std::move(title);
  UpdateTimestamp();
}

void Entity::SetTags(TagList tags) {
  meta_.tags = std::move(tags);
  UpdateTimestamp();
}

void Entity::RestoreStateFrom(const Entity& other) {
  meta_ = other.meta_;
  content_ = other.content_;
}

void Entity::UpdateTimestamp() { meta_.updated_at = Timestamp::Now(); }

void Entity::AddChild(std::unique_ptr<Entity> child) {
  if (!child) {
    throw LogicError(
        "Entity::AddChild: attempt to add a null pointer as a child");
  }

  if (child.get() == this) {
    throw LogicError("Entity::AddChild: an entity cannot be its own child");
  }

  Entity* current_ancestor = this->parent_;
  while (current_ancestor != nullptr) {
    if (current_ancestor->GetId() == child->GetId()) {
      throw CommandError(
          "Entity::AddChild: circular dependency detected. The target child is "
          "already an ancestor of this entity.");
    }
    current_ancestor = current_ancestor->parent_;
  }

  child->parent_ = this;
  children_.push_back(std::move(child));

  UpdateTimestamp();
}

std::unique_ptr<Entity> Entity::RemoveChild(const ID& id) {
  auto it = std::find_if(children_.begin(), children_.end(),
                         [&id](const std::unique_ptr<Entity>& child) {
                           return child->GetId() == id;
                         });

  if (it != children_.end()) {
    std::unique_ptr<Entity> removed = std::move(*it);
    removed->parent_ = nullptr;
    children_.erase(it);

    UpdateTimestamp();

    return removed;
  }
  return nullptr;
}

}  // namespace core::entities
