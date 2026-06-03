#pragma once

#include <future>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/commands/command_manager.hpp"
#include "app/commands/edit_action.hpp"
#include "core/entities/entity.hpp"
#include "core/entities/task.hpp"
#include "core/interfaces/storage.hpp"
#include "core/logic/registry.hpp"
#include "core/types/id.hpp"
#include "core/types/title.hpp"

namespace app {

class LifeEngine {
 public:
  explicit LifeEngine(std::unique_ptr<core::interfaces::IStorage> storage);
  ~LifeEngine();

  LifeEngine(const LifeEngine&) = delete;
  LifeEngine& operator=(const LifeEngine&) = delete;

  void Load();
  void SaveAll();
  void WaitAllSaves();

  core::ID CreateTask(const core::Title& title, const std::string& content);
  core::ID CreateNote(const core::Title& title, const std::string& content);

  void RemoveEntity(const core::ID& id);

  void MoveEntity(const core::ID& entity_id, std::optional<core::ID> parent_id);

  void EditEntity(const core::ID& id, commands::EditAction action);

  void Undo();
  void Redo();
  bool CanUndo() const;
  bool CanRedo() const;

  core::entities::Entity* GetEntity(const core::ID& id) const;

  std::vector<core::entities::Entity*> GetRootEntities() const;

  std::vector<core::entities::Entity*> FindByTag(const core::Tag& tag) const;

  std::vector<core::entities::Task*> GetTasks(
      std::optional<core::TaskStatus> status = std::nullopt) const;

  std::vector<core::entities::Entity*> Search(const std::string& query) const;

  core::ID ResolveId(const std::string& prefix) const;

 private:
  struct PendingSave {
    core::ID entity_id;
    std::future<void> future;
  };

  core::logic::Registry<core::entities::Entity> registry_;
  commands::CommandManager command_manager_;
  std::unique_ptr<core::interfaces::IStorage> storage_;

  std::vector<PendingSave> pending_saves_;

  static void SortByUpdateDate(std::vector<core::entities::Entity*>& list);
};

}  // namespace app
