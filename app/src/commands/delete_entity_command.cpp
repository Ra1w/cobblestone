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

  if (parent_id_.has_value()) {
    auto* parent = registry_.Get(*parent_id_);
    if (parent != nullptr) {
      parent->AddChild(std::move(entity_));
      return;
    }
  }

  registry_.Add(std::move(entity_));
}

}  // namespace app::commands
