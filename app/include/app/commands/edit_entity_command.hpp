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

  core::entities::Metadata memento_meta_;
  std::string memento_content_;
};

}  // namespace app::commands
