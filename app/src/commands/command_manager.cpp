#include "app/commands/command_manager.hpp"

namespace app::commands {

void CommandManager::Invoke(std::unique_ptr<ICommand> command) {
  if (!command) {
    return;
  }

  command->Execute();

  undo_stack_.push_back(std::move(command));
  if (undo_stack_.size() > MAX_HISTORY) {
    undo_stack_.pop_front();
  }

  redo_stack_.clear();
}

void CommandManager::Undo() {
  if (undo_stack_.empty()) {
    return;
  }

  auto command = std::move(undo_stack_.back());
  undo_stack_.pop_back();

  try {
    command->Undo();
    redo_stack_.push_back(std::move(command));
    if (redo_stack_.size() > MAX_HISTORY) {
      redo_stack_.pop_front();
    }
  } catch (...) {
    throw;
  }
}

void CommandManager::Redo() {
  if (redo_stack_.empty()) {
    return;
  }

  auto command = std::move(redo_stack_.back());
  redo_stack_.pop_back();

  try {
    command->Execute();
    undo_stack_.push_back(std::move(command));
    if (undo_stack_.size() > MAX_HISTORY) {
      undo_stack_.pop_front();
    }
  } catch (...) {
    throw;
  }
}

bool CommandManager::CanUndo() const { return !undo_stack_.empty(); }
bool CommandManager::CanRedo() const { return !redo_stack_.empty(); }

}  // namespace app::commands
