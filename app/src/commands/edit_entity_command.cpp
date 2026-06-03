#include "app/commands/edit_entity_command.hpp"
#include "core/exceptions.hpp"

namespace app::commands {

EditEntityCommand::EditEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry, const core::ID& id,
    EditAction action)
    : registry_(registry),
      id_(id),
      action_(std::move(action)) {}

void EditEntityCommand::Execute() {
  auto* entity = registry_.Get(id_);
  if (!entity) {
    throw core::NotFoundError(
        "EditEntityCommand: Entity not found for editing");
  }

  memento_ = entity->CloneWithoutChildren();
  bool was_dirty = entity->IsDirty();
  
  try {
    action_(*entity);
  } catch (...) {
    entity->RestoreStateFrom(*memento_);
    if (!was_dirty) {
      entity->ClearDirty();
    }
    throw;
  }
}

void EditEntityCommand::Undo() {
  auto* entity = registry_.Get(id_);
  if (entity == nullptr || memento_ == nullptr) {
    return;
  }

  entity->RestoreStateFrom(*memento_);
}

}  // namespace app::commands
