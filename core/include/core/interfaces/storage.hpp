#pragma once

#include <future>
#include <memory>
#include <vector>

#include "core/entities/entity.hpp"
#include "core/interfaces/persistence_entry.hpp"

namespace core::interfaces {

class IStorage {
 public:
  IStorage() = default;
  virtual ~IStorage() = default;

  IStorage(const IStorage&) = delete;
  IStorage& operator=(const IStorage&) = delete;
  IStorage(IStorage&&) = delete;
  IStorage& operator=(IStorage&&) = delete;

  virtual std::future<void> SaveAsync(const entities::Entity& entity) = 0;

  virtual std::vector<PersistenceEntry> LoadAll() = 0;

  virtual void Remove(const ID& id) = 0;
};

}  // namespace core::interfaces
