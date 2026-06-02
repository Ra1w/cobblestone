#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "core/entities/entity.hpp"
#include "core/exceptions.hpp"
#include "core/types/id.hpp"

namespace core::logic {

template <typename T>
concept Registrable = requires(T t) {
  { t.GetId() } -> std::convertible_to<const core::ID&>;
  {
    t.GetChildren()
  }
  -> std::same_as<const std::vector<std::unique_ptr<core::entities::Entity>>&>;
};

template <Registrable T>
class Registry {
 public:
  Registry() = default;
  ~Registry() = default;

  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;

  void Add(std::unique_ptr<T> item) {
    if (!item) {
      throw LogicError("Registry::Add: Attempt to add null pointer");
    }

    std::unique_lock lock(mutex_);
    const ID& id = item->GetId();

    if (all_entities_.contains(id)) {
      throw LogicError("Registry::Add: Entity with ID [" + id.Str() +
                       "] already exists");
    }

    RegisterRecursive(item.get());
    roots_[id] = std::move(item);
  }

  T* Get(const ID& id) const {
    std::shared_lock lock(mutex_);
    auto it = all_entities_.find(id);
    if (it != all_entities_.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::unique_ptr<T> Remove(const ID& id) {
    std::unique_lock lock(mutex_);

    auto it = all_entities_.find(id);
    if (it == all_entities_.end()) {
      throw NotFoundError("Registry::Remove: Entity [" + id.Str() +
                          "] not found");
    }

    T* target = it->second;
    UnregisterRecursive(target);

    auto* parent = target->GetParent();
    if (parent != nullptr) {
      auto removed_base = parent->RemoveChild(id);
      return std::unique_ptr<T>(static_cast<T*>(removed_base.release()));
    }

    auto root_it = roots_.find(id);
    if (root_it == roots_.end()) {
      throw LogicError(
          "Registry::Remove: Internal inconsistency (entity in index but not "
          "in roots)");
    }

    std::unique_ptr<T> removed = std::move(root_it->second);
    roots_.erase(root_it);
    return removed;
  }

  void MoveEntity(const ID& entity_id, std::optional<ID> new_parent_id) {
    std::unique_lock lock(mutex_);

    auto it = all_entities_.find(entity_id);
    if (it == all_entities_.end()) {
      throw NotFoundError("Registry::Move: Entity not found");
    }

    T* target = it->second;

    if (new_parent_id.has_value()) {
      auto parent_it = all_entities_.find(*new_parent_id);
      if (parent_it == all_entities_.end()) {
        throw NotFoundError("Registry::Move: Target parent not found");
      }

      T* new_parent = parent_it->second;
      core::entities::Entity* current = new_parent;
      while (current != nullptr) {
        if (current->GetId() == entity_id) {
          throw CommandError("Registry::Move: Circular dependency detected");
        }
        current = current->GetParent();
      }
    }

    std::unique_ptr<T> owned_target;
    if (auto* old_parent = target->GetParent()) {
      owned_target = std::unique_ptr<T>(
          static_cast<T*>(old_parent->RemoveChild(entity_id).release()));
    } else {
      auto root_it = roots_.find(entity_id);
      owned_target = std::move(root_it->second);
      roots_.erase(root_it);
    }

    if (new_parent_id.has_value()) {
      T* new_parent = all_entities_[*new_parent_id];
      new_parent->AddChild(std::move(owned_target));
    } else {
      roots_[entity_id] = std::move(owned_target);
    }
  }

  ID ResolveId(const std::string& prefix) const {
    std::shared_lock lock(mutex_);

    std::vector<ID> matches;
    for (const auto& [id, ptr] : all_entities_) {
      if (id.Str() == prefix) {
        return id;
      }
      if (id.Str().starts_with(prefix)) {
        matches.push_back(id);
      }
    }

    if (matches.empty()) {
      throw NotFoundError("Registry: No entity found with prefix [" + prefix +
                          "]");
    }

    if (matches.size() > 1) {
      std::string error_msg =
          "Registry: Ambiguous prefix [" + prefix + "]. Matches: ";
      for (const auto& m : matches) {
        error_msg += m.Str() + " ";
      }
      throw CommandError(error_msg);
    }

    return matches[0];
  }

  template <typename Predicate>
  std::vector<T*> FindIf(Predicate predicate) const {
    std::shared_lock lock(mutex_);
    std::vector<T*> results;

    for (const auto& [id, ptr] : all_entities_) {
      if (predicate(*ptr)) {
        results.push_back(ptr);
      }
    }
    return results;
  }

  size_t Count() const {
    std::shared_lock lock(mutex_);
    return all_entities_.size();
  }

  void Clear() {
    std::unique_lock lock(mutex_);
    roots_.clear();
    all_entities_.clear();
  }

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<ID, std::unique_ptr<T>> roots_;
  std::unordered_map<ID, T*> all_entities_;

  void RegisterRecursive(T* entity) {
    if (entity == nullptr) {
      return;
    }
    all_entities_[entity->GetId()] = entity;
    for (const auto& child : entity->GetChildren()) {
      RegisterRecursive(static_cast<T*>(child.get()));
    }
  }

  void UnregisterRecursive(T* entity) {
    if (entity == nullptr) {
      return;
    }
    for (const auto& child : entity->GetChildren()) {
      UnregisterRecursive(static_cast<T*>(child.get()));
    }
    all_entities_.erase(entity->GetId());
  }
};

}  // namespace core::logic
