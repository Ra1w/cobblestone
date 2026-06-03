#include "app/life_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <print>
#include <ranges>

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
    try {
      if (entry.parent_id.has_value()) {
        pending_links.push_back({entry.entity->GetId(), *entry.parent_id});
      }
      registry_.Add(std::move(entry.entity));
    } catch (const core::LogicError& e) {
      std::println(stderr, "Skipping corrupted/duplicate entry: {}", e.what());
    }
  }

  for (const auto& link : pending_links) {
    try {
      auto* child = registry_.Get(link.child_id);
      auto* parent = registry_.Get(link.parent_id);

      if (child != nullptr && parent != nullptr) {
        auto child_ts = child->GetMetadata().updated_at;
        auto parent_ts = parent->GetMetadata().updated_at;

        registry_.MoveEntity(link.child_id, link.parent_id);

        child->SetUpdatedAt(child_ts);
        parent->SetUpdatedAt(parent_ts);
      } else {
        registry_.MoveEntity(link.child_id, link.parent_id);
      }
    } catch (const core::CoreException& e) {
      std::println(
          stderr,
          "Warning: Failed to restore link for child [{}] to parent [{}]: {}",
          link.child_id.Str(), link.parent_id.Str(), e.what());
    }
  }

  for (auto* e : registry_.FindIf([](const auto&) { return true; })) {
    e->ClearDirty();
  }
}

void LifeEngine::SaveAll() {
  if (!storage_) {
    return;
  }

  std::erase_if(pending_saves_, [this](PendingSave& task) {
    if (task.future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready) {
      try {
        task.future.get();
      } catch (const std::exception& e) {
        std::println(stderr,
                     "Critical Error during background save for [{}]: {}",
                     task.entity_id.Str(), e.what());
        if (auto* e_ptr = registry_.Get(task.entity_id)) {
          e_ptr->ForceDirty();
        }
      }
      return true;
    }
    return false;
  });

  auto dirty_entities =
      registry_.FindIf([](const auto& e) { return e.IsDirty(); });

  for (auto* entity : dirty_entities) {
    pending_saves_.push_back({entity->GetId(), storage_->SaveAsync(*entity)});
    entity->ClearDirty();
  }
}

void LifeEngine::WaitAllSaves() {
  for (auto& task : pending_saves_) {
    if (task.future.valid()) {
      try {
        task.future.get();
      } catch (const std::exception& e) {
        std::println(stderr, "Critical Error waiting for save [{}]: {}",
                     task.entity_id.Str(), e.what());
        if (auto* e_ptr = registry_.Get(task.entity_id)) {
          e_ptr->ForceDirty();
        }
      }
    }
  }
  pending_saves_.clear();
}

core::ID LifeEngine::CreateTask(const core::Title& title,
                                const std::string& content) {
  auto task = std::make_unique<core::entities::Task>(
      core::entities::Metadata(core::ID::Generate(), title, {}), content);
  core::ID id = task->GetId();
  command_manager_.InvokeMake<commands::AddEntityCommand>(
      registry_, storage_.get(), std::move(task));
  SaveAll();
  return id;
}

core::ID LifeEngine::CreateNote(const core::Title& title,
                                const std::string& content) {
  auto note = std::make_unique<core::entities::Note>(
      core::entities::Metadata(core::ID::Generate(), title, {}), content);
  core::ID id = note->GetId();
  command_manager_.InvokeMake<commands::AddEntityCommand>(
      registry_, storage_.get(), std::move(note));
  SaveAll();
  return id;
}

void LifeEngine::RemoveEntity(const core::ID& id) {
  command_manager_.InvokeMake<commands::DeleteEntityCommand>(
      registry_, storage_.get(), id);
  SaveAll();
}

void LifeEngine::MoveEntity(const core::ID& entity_id,
                            std::optional<core::ID> parent_id) {
  command_manager_.Invoke(std::make_unique<commands::MoveEntityCommand>(
      registry_, entity_id, parent_id));
  SaveAll();
}

void LifeEngine::EditEntity(const core::ID& id, commands::EditAction action) {
  command_manager_.Invoke(std::make_unique<commands::EditEntityCommand>(
      registry_, id, std::move(action)));
  SaveAll();
}

void LifeEngine::Undo() {
  command_manager_.Undo();
  SaveAll();
}

void LifeEngine::Redo() {
  command_manager_.Redo();
  SaveAll();
}

bool LifeEngine::CanUndo() const { return command_manager_.CanUndo(); }

bool LifeEngine::CanRedo() const { return command_manager_.CanRedo(); }

core::entities::Entity* LifeEngine::GetEntity(const core::ID& id) const {
  return registry_.Get(id);
}

void LifeEngine::SortByUpdateDate(std::vector<core::entities::Entity*>& list) {
  std::sort(list.begin(), list.end(), [](const auto* a, const auto* b) {
    return a->GetMetadata().updated_at > b->GetMetadata().updated_at;
  });
}

std::vector<core::entities::Entity*> LifeEngine::GetRootEntities() const {
  auto roots = registry_.FindIf(
      [](const core::entities::Entity& e) { return e.GetParent() == nullptr; });
  SortByUpdateDate(roots);
  return roots;
}

std::vector<core::entities::Entity*> LifeEngine::FindByTag(
    const core::Tag& tag) const {
  auto results = registry_.FindIf([&tag](const core::entities::Entity& e) {
    return std::ranges::contains(e.GetMetadata().tags, tag);
  });
  SortByUpdateDate(results);
  return results;
}

std::vector<core::entities::Task*> LifeEngine::GetTasks(
    std::optional<core::TaskStatus> status) const {
  auto entities = registry_.FindIf([status](const core::entities::Entity& e) {
    if (e.GetType() != core::EntityType::Task) {
      return false;
    }

    if (status.has_value()) {
      return static_cast<const core::entities::Task&>(e).GetStatus() == *status;
    }

    return true;
  });

  SortByUpdateDate(entities);

  auto tasks = entities | std::views::transform([](auto* e) {
                 return static_cast<core::entities::Task*>(e);
               }) |
               std::ranges::to<std::vector>();

  return tasks;
}

std::vector<core::entities::Entity*> LifeEngine::Search(
    const std::string& query) const {
  if (query.empty()) {
    return {};
  }

  auto case_insensitive_equals = [](unsigned char ch1, unsigned char ch2) {
    return std::tolower(ch1) == std::tolower(ch2);
  };

  auto results = registry_.FindIf([&query, &case_insensitive_equals](
                                      const core::entities::Entity& e) {
    const std::string& title = e.GetMetadata().title.Str();

    auto title_it = std::search(title.begin(), title.end(), query.begin(),
                                query.end(), case_insensitive_equals);
    if (title_it != title.end()) {
      return true;
    }

    const std::string& content = e.GetContent();
    auto content_it = std::search(content.begin(), content.end(), query.begin(),
                                  query.end(), case_insensitive_equals);
    if (content_it != content.end()) {
      return true;
    }

    return false;
  });

  SortByUpdateDate(results);
  return results;
}

core::ID LifeEngine::ResolveId(const std::string& prefix) const {
  return registry_.ResolveId(prefix);
}

}  // namespace app
