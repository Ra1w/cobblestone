#pragma once

namespace app::commands {

class ICommand {
 public:
  ICommand() = default;
  virtual ~ICommand() = default;

  ICommand(const ICommand&) = delete;
  ICommand& operator=(const ICommand&) = delete;
  ICommand(ICommand&&) = delete;
  ICommand& operator=(ICommand&&) = delete;

  virtual void Execute() = 0;

  virtual void Undo() = 0;
};

}  // namespace app::commands
