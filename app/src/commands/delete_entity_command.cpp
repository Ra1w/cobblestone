#include "app/commands/delete_entity_command.hpp"

namespace app::commands {

DeleteEntityCommand::DeleteEntityCommand(
    core::logic::Registry<core::entities::Entity>& registry, const core::ID& id)
    : registry_(registry), id_(id) {}

void DeleteEntityCommand::Execute() { entity_ = registry_.Remove(id_); }

void DeleteEntityCommand::Undo() { registry_.Add(std::move(entity_)); }

}  // namespace app::commands
