#pragma once

#include <functional>

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

using EditAction = std::function<void(core::entities::Entity&)>;

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
