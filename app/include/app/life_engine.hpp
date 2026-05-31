#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/commands/command_manager.hpp"
#include "app/commands/edit_action.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"
#include "core/types/id.hpp"
#include "core/types/title.hpp"

namespace app {

class LifeEngine {
 public:
  LifeEngine();
  ~LifeEngine() = default;

  LifeEngine(const LifeEngine&) = delete;
  LifeEngine& operator=(const LifeEngine&) = delete;

  void CreateTask(const core::Title& title, const std::string& content);
  void CreateNote(const core::Title& title, const std::string& content);

  void RemoveEntity(const core::ID& id);

  void MoveEntity(const core::ID& entity_id, std::optional<core::ID> parent_id);

  void EditEntity(const core::ID& id, commands::EditAction action);

  void Undo();
  void Redo();
  bool CanUndo() const;
  bool CanRedo() const;

  core::entities::Entity* GetEntity(const core::ID& id) const;

  std::vector<core::entities::Entity*> GetRootEntities() const;

 private:
  core::logic::Registry<core::entities::Entity> registry_;
  commands::CommandManager command_manager_;
};

}  // namespace app
