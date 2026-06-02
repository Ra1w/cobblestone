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

  if (auto* parent = entity->GetParent()) {
    old_parent_id_ = parent->GetId();
  }
}

void MoveEntityCommand::Execute() {
  registry_.MoveEntity(entity_id_, new_parent_id_);
}

void MoveEntityCommand::Undo() {
  registry_.MoveEntity(entity_id_, old_parent_id_);
}

}  // namespace app::commands
