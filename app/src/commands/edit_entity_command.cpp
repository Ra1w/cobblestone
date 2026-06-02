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
  if (!entity) {
    return;
  }

  entity->SetTitle(memento_meta_.title);
  entity->SetTags(memento_meta_.tags);
  entity->SetContent(memento_content_);
}

}  // namespace app::commands
