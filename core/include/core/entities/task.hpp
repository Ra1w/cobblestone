#pragma once

#include "core/entities/entity.hpp"

namespace core::entities {

enum class TaskStatus { Todo, InProgress, Done };

class Task : public Entity {
 public:
  Task(Metadata meta, std::string content = "",
       TaskStatus status = TaskStatus::Todo);

  EntityType GetType() const override { return EntityType::Task; }
  std::unique_ptr<Entity> Clone() const override;

  TaskStatus GetStatus() const { return status_; }
  void SetStatus(TaskStatus status);

 private:
  TaskStatus status_;
};

}  // namespace core::entities