#include "core/entities/task.hpp"

namespace core::entities {

Task::Task(Metadata meta, std::string content, TaskStatus status)
    : Entity(std::move(meta), std::move(content)), status_(status) {}

void Task::SetStatus(TaskStatus status) {
  status_ = status;
  UpdateTimestamp();
}

std::unique_ptr<Entity> Task::Clone() const {
  auto clone = std::make_unique<Task>(meta_, content_, status_);

  for (const auto& child : children_) {
    clone->AddChild(child->Clone());
  }

  return clone;
}

std::unique_ptr<Entity> Task::CloneWithoutChildren() const {
  return std::make_unique<Task>(meta_, content_, status_);
}

void Task::RestoreStateFrom(const Entity& other) {
  Entity::RestoreStateFrom(other);
  if (other.GetType() == EntityType::Task) {
    status_ = static_cast<const Task&>(other).status_;
  }
}

}  // namespace core::entities
