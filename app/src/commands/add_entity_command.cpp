#include "app/commands/add_entity_command.hpp"
#include "core/exceptions.hpp"

namespace app::commands {

AddEntityCommand::AddEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry,
    std::unique_ptr<core::entities::Entity> entity)
    : registry_(registry), id_(entity ? entity->GetId() : core::ID("")) {
  if (!entity) {
    throw core::LogicError(
        "AddEntityCommand: Received null entity in constructor");
  }
  entity_ = std::move(entity);
}

void AddEntityCommand::Execute() {
  if (!entity_) {
    throw core::LogicError(
        "AddEntityCommand: Entity is null (already executed?)");
  }
  registry_.Add(std::move(entity_));
}

void AddEntityCommand::Undo() { entity_ = registry_.Remove(id_); }

}  // namespace app::commands
