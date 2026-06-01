#include "app/life_engine.hpp"

#include <algorithm>
#include <chrono>

#include "app/commands/add_entity_command.hpp"
#include "app/commands/delete_entity_command.hpp"
#include "app/commands/edit_entity_command.hpp"
#include "app/commands/move_entity_command.hpp"
#include "core/entities/note.hpp"
#include "core/entities/task.hpp"
#include "core/exceptions.hpp"

namespace app {

LifeEngine::LifeEngine(std::unique_ptr<core::interfaces::IStorage> storage)
    : storage_(std::move(storage)) {}

LifeEngine::~LifeEngine() { WaitAllSaves(); }

void LifeEngine::Load() {
  if (!storage_) {
    return;
  }

  auto entries = storage_->LoadAll();

  struct Link {
    core::ID child_id;
    core::ID parent_id;
  };

  std::vector<Link> pending_links;

  for (auto& entry : entries) {
    if (entry.parent_id.has_value()) {
      pending_links.push_back({entry.entity->GetId(), *entry.parent_id});
    }
    registry_.Add(std::move(entry.entity));
  }

  for (const auto& link : pending_links) {
    try {
      auto* parent = registry_.Get(link.parent_id);
      if (parent != nullptr) {
        auto child = registry_.Remove(link.child_id);
        parent->AddChild(std::move(child));
      }
    } catch (const core::CoreException&) {
    }
  }
}

void LifeEngine::SaveAll() {
  if (!storage_) {
    return;
  }

  std::erase_if(pending_saves_, [](std::future<void>& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  });

  auto roots = GetRootEntities();
  for (auto* root : roots) {
    SaveRecursive(*root);
  }
}

void LifeEngine::SaveRecursive(const core::entities::Entity& entity) {
  pending_saves_.push_back(storage_->SaveAsync(entity));

  for (const auto& child : entity.GetChildren()) {
    SaveRecursive(*child);
  }
}

void LifeEngine::WaitAllSaves() {
  for (auto& f : pending_saves_) {
    if (f.valid()) {
      f.get();
    }
  }
  pending_saves_.clear();
}

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
  if (storage_) {
    storage_->Remove(id);
  }

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

core::ID LifeEngine::ResolveId(const std::string& prefix) const {
  return registry_.ResolveId(prefix);
}

}  // namespace app
