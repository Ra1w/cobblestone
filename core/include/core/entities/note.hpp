#pragma once

#include "core/entities/entity.hpp"

namespace core::entities {

class Note final : public Entity {
 public:
  using Entity::Entity;

  EntityType GetType() const override { return EntityType::Note; }
  std::unique_ptr<Entity> Clone() const override;

  std::unique_ptr<Entity> CloneWithoutChildren() const override;
};

}  // namespace core::entities
