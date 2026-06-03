#pragma once

#include <memory>
#include <optional>

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

class DeleteEntityCommand final : public ICommand {
 public:
  DeleteEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                      const core::ID& id);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  std::unique_ptr<core::entities::Entity> entity_;
  core::ID id_;
  std::optional<core::ID> parent_id_;
};

}  // namespace app::commands
