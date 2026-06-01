#pragma once

#include <string>
#include <vector>

#include "app/life_engine.hpp"

namespace cli {

class ConsoleApp {
 public:
  explicit ConsoleApp(app::LifeEngine& engine);
  ~ConsoleApp() = default;

  void Run();

 private:
  app::LifeEngine& engine_;
  bool running_ = true;

  void ProcessInput(const std::string& input);

  void ShowHelp();
  void ListRoot();
  void CreateTask();
  void CreateNote();
  void DeleteEntity();
  void MoveEntity();
  void Undo();
  void Redo();

  void PrintTree(const core::entities::Entity& entity, int depth);
};

}  // namespace cli
