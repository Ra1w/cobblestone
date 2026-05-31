#include "app/commands/move_entity_command.hpp"

#include "core/exceptions.hpp"

namespace app::commands {

MoveEntityCommand::MoveEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry,
    const core::ID& entity_id, std::optional<core::ID> new_parent_id)
    : registry_(registry),
      entity_id_(entity_id),
      new_parent_id_(new_parent_id) {
  auto* entity = registry_.FindDeep(entity_id_);
  if (!entity) {
    throw core::NotFoundError("MoveCommand: Entity [" + entity_id.Str() +
                              "] not found");
  }

  if (new_parent_id && *new_parent_id == entity_id_) {
    throw core::CommandError("MoveCommand: Cannot move entity into itself");
  }

  if (new_parent_id && IsDescendantOf(*entity, *new_parent_id)) {
    throw core::CommandError(
        "MoveCommand: Circular dependency. Target parent is a descendant of "
        "the moved entity");
  }

  auto* parent = entity->GetParent();
  if (parent) {
    old_parent_id_ = parent->GetId();
  }
}

void MoveEntityCommand::Execute() {
  auto entity = Detach(entity_id_);
  if (!entity) {
    throw core::LogicError("MoveCommand: Entity vanished during Execute");
  }
  Attach(std::move(entity), new_parent_id_);
}

void MoveEntityCommand::Undo() {
  auto entity = Detach(entity_id_);
  if (!entity) {
    throw core::LogicError("MoveCommand: Entity vanished during Undo");
  }
  Attach(std::move(entity), old_parent_id_);
}

std::unique_ptr<core::entities::Entity> MoveEntityCommand::Detach(
    const core::ID& id) {
  auto* entity = registry_.FindDeep(id);
  if (!entity) {
    return nullptr;
  }

  auto* parent = entity->GetParent();
  if (parent) {
    return parent->RemoveChild(id);
  }
  return registry_.Remove(id);
}

void MoveEntityCommand::Attach(std::unique_ptr<core::entities::Entity> entity,
                               std::optional<core::ID> parent_id) {
  if (parent_id) {
    auto* parent = registry_.FindDeep(*parent_id);
    if (!parent) {
      registry_.Add(std::move(entity));
      return;
    }
    parent->AddChild(std::move(entity));
  } else {
    registry_.Add(std::move(entity));
  }
}

bool MoveEntityCommand::IsDescendantOf(const core::entities::Entity& root,
                                       const core::ID& target_id) const {
  for (const auto& child : root.GetChildren()) {
    if (child->GetId() == target_id) {
      return true;
    }
    if (IsDescendantOf(*child, target_id)) {
      return true;
    }
  }
  return false;
}

}  // namespace app::commands
