#pragma once

namespace app::commands {

class ICommand {
 public:
  virtual ~ICommand() = default;

  virtual void Execute() = 0;

  virtual void Undo() = 0;
};

}  // namespace app::commands