#pragma once

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

class AddEntityCommand : public ICommand {
 public:
  AddEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                   std::unique_ptr<core::entities::Entity> entity);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  std::unique_ptr<core::entities::Entity> entity_;
  core::ID id_;
};

}  // namespace app::commands
