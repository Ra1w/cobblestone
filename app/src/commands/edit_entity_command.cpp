#include "app/commands/edit_entity_command.hpp"

#include "core/exceptions.hpp"

namespace app::commands {

EditEntityCommand::EditEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry, const core::ID& id,
    EditAction action)
    : registry_(registry),
      id_(id),
      action_(std::move(action)),
      memento_meta_(core::ID("temp"), core::Title("temp"), {}) {}

void EditEntityCommand::Execute() {
  auto* entity = registry_.Get(id_);
  if (!entity) {
    throw core::NotFoundError(
        "EditEntityCommand: Entity not found for editing");
  }

  memento_meta_ = entity->GetMetadata();
  memento_content_ = entity->GetContent();
  action_(*entity);
}

void EditEntityCommand::Undo() {
  auto* entity = registry_.Get(id_);
  if (entity == nullptr) {
    return;
  }

  entity->RestoreState(memento_meta_.title, memento_meta_.tags,
                       memento_content_, memento_meta_.updated_at);
}

}  // namespace app::commands
