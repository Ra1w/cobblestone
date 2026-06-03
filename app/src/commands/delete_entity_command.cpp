#include "app/commands/delete_entity_command.hpp"

namespace app::commands {

DeleteEntityCommand::DeleteEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry,
    core::interfaces::IStorage* storage, const core::ID& id)
    : registry_(registry), storage_(storage), id_(id) {}

void DeleteEntityCommand::Execute() {
  parent_id_ = std::nullopt;
  removed_ids_.clear();

  auto* target = registry_.Get(id_);
  if (target != nullptr) {
    if (target->GetParent() != nullptr) {
      parent_id_ = target->GetParent()->GetId();
    }

    for (auto* e : target->WalkTree()) {
      removed_ids_.push_back(e->GetId());
    }
  }

  entity_ = registry_.Remove(id_);

  if (storage_ != nullptr) {
    for (const auto& r_id : removed_ids_) {
      storage_->Remove(r_id);
    }
  }
}

void DeleteEntityCommand::Undo() {
  if (!entity_) {
    return;
  }

  core::ID restored_id = entity_->GetId();

  for (auto* e : entity_->WalkTree()) {
    e->MarkDirty();
  }

  try {
    registry_.Add(std::move(entity_));
  } catch (const core::LogicError&) {
    return;
  }

  if (parent_id_.has_value()) {
    try {
      registry_.MoveEntity(restored_id, *parent_id_);
    } catch (const core::NotFoundError&) {
    }
  }
}

}  // namespace app::commands
