#include "app/commands/move_entity_command.hpp"

#include "core/exceptions.hpp"

namespace app::commands {

MoveEntityCommand::MoveEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry,
    const core::ID& entity_id, std::optional<core::ID> new_parent_id)
    : registry_(registry),
      entity_id_(entity_id),
      new_parent_id_(new_parent_id) {
  auto* entity = registry_.Get(entity_id_);
  if (entity == nullptr) {
    throw core::NotFoundError("MoveCommand: Target entity [" + entity_id.Str() +
                              "] not found");
  }

  if (new_parent_id.has_value()) {
    if (*new_parent_id == entity_id_) {
      throw core::CommandError("MoveCommand: Cannot move entity into itself");
    }

    auto* parent = registry_.Get(*new_parent_id);
    if (parent == nullptr) {
      throw core::NotFoundError("MoveCommand: Target parent [" +
                                new_parent_id->Str() + "] not found");
    }
  }

  auto* current_parent = entity->GetParent();
  if (current_parent != nullptr) {
    old_parent_id_ = current_parent->GetId();
  }
}

void MoveEntityCommand::Execute() {
  auto entity = Detach(entity_id_);
  if (!entity) {
    throw core::LogicError("MoveCommand: Entity vanished during execution");
  }

  try {
    Attach(std::move(entity), new_parent_id_);
  } catch (const core::CoreException& e) {
    throw;
  }
}

void MoveEntityCommand::Undo() {
  auto entity = Detach(entity_id_);
  if (!entity) {
    throw core::LogicError(
        "MoveCommand: Critical failure during Undo (entity missing)");
  }
  Attach(std::move(entity), old_parent_id_);
}

std::unique_ptr<core::entities::Entity> MoveEntityCommand::Detach(
    const core::ID& id) {
  return registry_.Remove(id);
}

void MoveEntityCommand::Attach(std::unique_ptr<core::entities::Entity> entity,
                               std::optional<core::ID> parent_id) {
  if (parent_id.has_value()) {
    auto* parent = registry_.Get(*parent_id);
    if (parent == nullptr) {
      registry_.Add(std::move(entity));
      return;
    }
    parent->AddChild(std::move(entity));
  } else {
    registry_.Add(std::move(entity));
  }
}

}  // namespace app::commands
