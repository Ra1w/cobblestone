#include "app/life_engine.hpp"

#include "app/commands/add_entity_command.hpp"
#include "app/commands/delete_entity_command.hpp"
#include "app/commands/edit_entity_command.hpp"
#include "app/commands/move_entity_command.hpp"
#include "core/entities/note.hpp"
#include "core/entities/task.hpp"

namespace app {

LifeEngine::LifeEngine() : registry_(), command_manager_() {}

void LifeEngine::CreateTask(const core::Title& title,
                            const std::string& content) {
  auto task = std::make_unique<core::entities::Task>(
      core::entities::Metadata(core::ID::Generate(), title, {}), content);

  command_manager_.Invoke(
      std::make_unique<commands::AddEntityCommand>(registry_, std::move(task)));
}

void LifeEngine::CreateNote(const core::Title& title,
                            const std::string& content) {
  auto note = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID::Generate(), title, {}), content);

  command_manager_.Invoke(
      std::make_unique<commands::AddEntityCommand>(registry_, std::move(note)));
}

void LifeEngine::RemoveEntity(const core::ID& id) {
  command_manager_.Invoke(
      std::make_unique<commands::DeleteEntityCommand>(registry_, id));
}

void LifeEngine::MoveEntity(const core::ID& entity_id,
                            std::optional<core::ID> parent_id) {
  command_manager_.Invoke(std::make_unique<commands::MoveEntityCommand>(
      registry_, entity_id, parent_id));
}

void LifeEngine::EditEntity(const core::ID& id, commands::EditAction action) {
  command_manager_.Invoke(std::make_unique<commands::EditEntityCommand>(
      registry_, id, std::move(action)));
}

void LifeEngine::Undo() { command_manager_.Undo(); }

void LifeEngine::Redo() { command_manager_.Redo(); }

bool LifeEngine::CanUndo() const { return command_manager_.CanUndo(); }

bool LifeEngine::CanRedo() const { return command_manager_.CanRedo(); }

core::entities::Entity* LifeEngine::GetEntity(const core::ID& id) const {
  return registry_.FindDeep(id);
}

std::vector<core::entities::Entity*> LifeEngine::GetRootEntities() const {
  return registry_.FindIf(
      [](const core::entities::Entity& e) { return e.GetParent() == nullptr; });
}

}  // namespace app
