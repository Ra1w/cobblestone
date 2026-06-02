#pragma once

#include <memory>
#include <deque>

#include "app/commands/command.hpp"

namespace app::commands {

class CommandManager {
 public:
  CommandManager() = default;
  ~CommandManager() = default;

  CommandManager(const CommandManager&) = delete;
  CommandManager& operator=(const CommandManager&) = delete;

  void Invoke(std::unique_ptr<ICommand> command);
  void Undo();
  void Redo();

  bool CanUndo() const;
  bool CanRedo() const;

 private:
  static constexpr size_t MAX_HISTORY = 100;

  std::deque<std::unique_ptr<ICommand>> undo_stack_;
  std::deque<std::unique_ptr<ICommand>> redo_stack_;
};

}  // namespace app::commands
