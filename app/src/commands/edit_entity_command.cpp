#include "app/commands/edit_entity_command.hpp"
#include "core/exceptions.hpp"

namespace app::commands {

EditEntityCommand::EditEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry, const core::ID& id,
    EditAction action)
    : registry_(registry), id_(id), action_(std::move(action)) {}

void EditEntityCommand::Execute() {
  auto* entity = registry_.Get(id_);
  if (!entity) {
    throw core::NotFoundError(
        "EditEntityCommand: Entity not found for editing");
  }

  memento_ = entity->Clone();
  action_(*entity);
}

void EditEntityCommand::Undo() {
  if (!memento_) {
    return;
  }

  registry_.Remove(id_);
  registry_.Add(std::move(memento_));
}

}  // namespace app::commands
