#include "app/commands/command_manager.hpp"

namespace app::commands {

void CommandManager::Invoke(std::unique_ptr<ICommand> command) {
  if (!command) {
    return;
  }

  command->Execute();
  undo_stack_.push(std::move(command));

  while (!redo_stack_.empty()) {
    redo_stack_.pop();
  }
}

void CommandManager::Undo() {
  if (undo_stack_.empty()) {
    return;
  }

  auto command = std::move(undo_stack_.top());
  undo_stack_.pop();

  command->Undo();
  redo_stack_.push(std::move(command));
}

void CommandManager::Redo() {
  if (redo_stack_.empty()) {
    return;
  }

  auto command = std::move(redo_stack_.top());
  redo_stack_.pop();

  command->Execute();
  undo_stack_.push(std::move(command));
}

bool CommandManager::CanUndo() const { return !undo_stack_.empty(); }
bool CommandManager::CanRedo() const { return !redo_stack_.empty(); }

}  // namespace app::commands