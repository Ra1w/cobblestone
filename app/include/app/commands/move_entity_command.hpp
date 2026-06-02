#pragma once

#include <optional>

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

class MoveEntityCommand : public ICommand {
 public:
  MoveEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                    const core::ID& entity_id,
                    std::optional<core::ID> new_parent_id);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  core::ID entity_id_;
  std::optional<core::ID> old_parent_id_;
  std::optional<core::ID> new_parent_id_;
};

}  // namespace app::commands
