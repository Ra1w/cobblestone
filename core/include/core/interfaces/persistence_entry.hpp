#pragma once

#include <memory>
#include <optional>

#include "core/entities/entity.hpp"
#include "core/types/id.hpp"

namespace core::interfaces {

struct PersistenceEntry {
  std::unique_ptr<entities::Entity> entity;

  std::optional<ID> parent_id;
};

}  // namespace core::interfaces
