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
  std::unique_ptr<core::entities::Entity> Detach(const core::ID& id);

  void Attach(std::unique_ptr<core::entities::Entity> entity,
              std::optional<core::ID> parent_id);

  bool IsDescendantOf(const core::entities::Entity& root,
                      const core::ID& target_id) const;

  core::logic::Registry<core::entities::Entity>& registry_;
  core::ID entity_id_;
  std::optional<core::ID> old_parent_id_;
  std::optional<core::ID> new_parent_id_;
};

}  // namespace app::commands
