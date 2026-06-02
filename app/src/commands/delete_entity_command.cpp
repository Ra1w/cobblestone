#include "app/commands/delete_entity_command.hpp"

namespace app::commands {

DeleteEntityCommand::DeleteEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry, const core::ID& id)
    : registry_(registry), id_(id) {}

void DeleteEntityCommand::Execute() {
  parent_id_ = std::nullopt;

  auto* target = registry_.Get(id_);
  if (target != nullptr) {
    if (target->GetParent() != nullptr) {
      parent_id_ = target->GetParent()->GetId();
    }
  }

  entity_ = registry_.Remove(id_);
}

void DeleteEntityCommand::Undo() {
  if (!entity_) {
    return;
  }

  core::ID restored_id = entity_->GetId();
  
  registry_.Add(std::move(entity_));

  if (parent_id_.has_value()) {
    registry_.MoveEntity(restored_id, *parent_id_);
  }
}

}  // namespace app::commands
