#pragma once

#include <string>

#include "app/life_engine.hpp"

namespace cli {

class ConsoleApp {
 public:
  explicit ConsoleApp(app::LifeEngine& engine);
  ~ConsoleApp() = default;

  ConsoleApp(const ConsoleApp&) = delete;
  ConsoleApp& operator=(const ConsoleApp&) = delete;

  void Run();

 private:
  app::LifeEngine& engine_;
  bool running_ = true;

  void ProcessInput(const std::string& input);

  void ShowHelp();

  void ListRoot();
  void ListTasks();

  void CreateTask();
  void CreateNote();

  void DeleteEntity();
  void MoveEntity();

  void Undo();
  void Redo();

  void ViewEntity();

  void EditContent();
  void AppendContent();
  void RenameEntity();
  void SetStatus();

  void AddTag();
  void RemoveTag();
  void FindByTag();

  void SearchText();

  void PrintTree(const core::entities::Entity& entity, int depth);

  core::TaskStatus ParseStatus(const std::string& status_str);
};

}  // namespace cli
