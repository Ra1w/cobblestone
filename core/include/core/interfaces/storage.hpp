#pragma once

#include <future>
#include <memory>
#include <vector>

#include "core/entities/entity.hpp"

namespace core::interfaces {

class IStorage {
 public:
  virtual ~IStorage() = default;

  virtual std::future<void> SaveAsync(const entities::Entity& entity) = 0;

  virtual std::vector<std::unique_ptr<entities::Entity>> LoadAll() = 0;

  virtual void Remove(const ID& id) = 0;
};

}  // namespace core::interfaces
