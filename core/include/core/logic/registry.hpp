#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "core/entities/entity.hpp"
#include "core/exceptions.hpp"
#include "core/types/id.hpp"

namespace core::logic {

template <typename T>
class Registry {
 public:
  Registry() = default;

  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;

  void Add(std::unique_ptr<T> item) {
    if (!item) {
      throw LogicError("Registry::Add: Attempt to add null pointer");
    }

    std::unique_lock lock(mutex_);
    const ID& id = item->GetId();

    if (items_.find(id) != items_.end()) {
      throw LogicError("Registry::Add: Entity with ID [" + id.Str() +
                       "] already exists");
    }

    items_[id] = std::move(item);
  }

  T* Get(const ID& id) const {
    std::shared_lock lock(mutex_);
    auto it = items_.find(id);
    if (it != items_.end()) {
      return it->second.get();
    }
    return nullptr;
  }

  std::unique_ptr<T> Remove(const ID& id) {
    std::unique_lock lock(mutex_);

    T* target = FindDeepInternal(id);
    if (!target) {
      throw NotFoundError("Registry::Remove: Entity [" + id.Str() +
                          "] not found");
    }

    auto* parent = target->GetParent();
    if (parent != nullptr) {
      return parent->RemoveChild(id);
    }

    auto it = items_.find(id);
    if (it == items_.end()) {
      throw LogicError("Registry::Remove: Internal inconsistency error");
    }

    std::unique_ptr<T> removed = std::move(it->second);
    items_.erase(it);
    return removed;
  }

  template <typename Predicate>
  std::vector<T*> FindIf(Predicate predicate) const {
    std::shared_lock lock(mutex_);
    std::vector<T*> results;

    for (const auto& [id, item] : items_) {
      if (predicate(*item)) {
        results.push_back(item.get());
      }
    }
    return results;
  }

  T* FindDeep(const ID& id) const {
    std::shared_lock lock(mutex_);
    return FindDeepInternal(id);
  }

  size_t Count() const {
    std::shared_lock lock(mutex_);
    return items_.size();
  }

  void Clear() {
    std::unique_lock lock(mutex_);
    items_.clear();
  }

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<ID, std::unique_ptr<T>> items_;

  T* FindDeepInternal(const ID& id) const {
    auto it = items_.find(id);
    if (it != items_.end()) {
      return it->second.get();
    }

    for (const auto& [root_id, item] : items_) {
      T* found = FindRecursive(item.get(), id);
      if (found) {
        return found;
      }
    }
    return nullptr;
  }

  T* FindRecursive(T* current, const ID& id) const {
    if (current->GetId() == id) {
      return dynamic_cast<T*>(current);
    }
    for (const auto& child : current->GetChildren()) {
      T* found = FindRecursive(child.get(), id);
      if (found) {
        return found;
      }
    }
    return nullptr;
  }
};

}  // namespace core::logic
