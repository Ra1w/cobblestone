#pragma once

#include "app/commands/command.hpp"
#include "core/entities/entity.hpp"
#include "core/interfaces/storage.hpp"
#include "core/logic/registry.hpp"

namespace app::commands {

class AddEntityCommand final : public ICommand {
 public:
  AddEntityCommand(core::logic::Registry<core::entities::Entity>& registry,
                   core::interfaces::IStorage* storage,
                   std::unique_ptr<core::entities::Entity> entity);

  void Execute() override;
  void Undo() override;

 private:
  core::logic::Registry<core::entities::Entity>& registry_;
  core::interfaces::IStorage* storage_;
  std::unique_ptr<core::entities::Entity> entity_;
  core::ID id_;
};

}  // namespace app::commands
