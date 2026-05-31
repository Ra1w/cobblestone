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

}  // namespace core::entities
