#pragma once

#include "app/commands/command.hpp"
#include "app/commands/edit_action.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"
#include "core/types/id.hpp"

namespace app::commands {

class EditEntityCommand : public ICommand {
 public:
  EditEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                    const core::ID& id, EditAction action);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  core::ID id_;
  EditAction action_;
  std::unique_ptr<core::entities::Entity> memento_;
};

}  // namespace app::commands
