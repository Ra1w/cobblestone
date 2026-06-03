#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/interfaces/storage.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

class DeleteEntityCommand final : public ICommand {
 public:
  DeleteEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                      core::interfaces::IStorage* storage, const core::ID& id);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  core::interfaces::IStorage* storage_;
  std::unique_ptr<core::entities::Entity> entity_;
  core::ID id_;
  std::optional<core::ID> parent_id_;
  std::vector<core::ID> removed_ids_;
};

}  // namespace app::commands
