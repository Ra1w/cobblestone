#include "cli/console_app.hpp"

#include <iostream>
#include <sstream>
#include <format>

#include "core/exceptions.hpp"

namespace cli {

ConsoleApp::ConsoleApp(app::LifeEngine& engine) : engine_(engine) {}

void ConsoleApp::Run() {
  engine_.Load();
  std::cout << "Life Gamificator 2026 Engine Loaded.\n";
  std::cout << "Type 'help' for commands list.\n";

  std::string line;
  while (running_) {
    std::cout << "\n> ";
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (line.empty()) {
      continue;
    }
    ProcessInput(line);
  }
}

void ConsoleApp::ProcessInput(const std::string& input) {
  try {
    if (input == "exit" || input == "quit") {
      engine_.SaveAll();
      engine_.WaitAllSaves();
      running_ = false;
    } else if (input == "help") {
      ShowHelp();
    } else if (input == "ls") {
      ListRoot();
    } else if (input == "add-t") {
      CreateTask();
    } else if (input == "add-n") {
      CreateNote();
    } else if (input == "rm") {
      DeleteEntity();
    } else if (input == "mv") {
      MoveEntity();
    } else if (input == "undo") {
      Undo();
    } else if (input == "redo") {
      Redo();
    } else if (input == "save") {
      engine_.SaveAll();
      std::cout << "Save initiated...\n";
    } else {
      std::cout << "Unknown command. Type 'help'.\n";
    }
  } catch (const core::CoreException& e) {
    std::cout << std::format("Error: {}\n", e.what());
  }
}

void ConsoleApp::ShowHelp() {
  std::cout << "Available commands:\n"
            << "  ls      - List all root entities (tree view)\n"
            << "  add-t   - Create a new Task\n"
            << "  add-n   - Create a new Note\n"
            << "  rm      - Remove entity by ID\n"
            << "  mv      - Move entity to a new parent\n"
            << "  undo    - Undo last action\n"
            << "  redo    - Redo last undone action\n"
            << "  save    - Force save all to disk\n"
            << "  exit    - Save and exit\n";
}

void ConsoleApp::ListRoot() {
  auto roots = engine_.GetRootEntities();
  if (roots.empty()) {
    std::cout << "Registry is empty.\n";
    return;
  }

  for (const auto* root : roots) {
    PrintTree(*root, 0);
  }
}

void ConsoleApp::PrintTree(const core::entities::Entity& entity, int depth) {
  std::string indent(depth * 2, ' ');
  std::string type_mark = (entity.GetType() == core::EntityType::Task) ? "[T]" : "[N]";
  
  std::cout << std::format("{}|-- {} {} (ID: {})\n", 
                           indent, type_mark, entity.GetMetadata().title.Str(), entity.GetId().Str());

  for (const auto& child : entity.GetChildren()) {
    PrintTree(*child, depth + 1);
  }
}

void ConsoleApp::CreateTask() {
  std::string title_raw, content;
  std::cout << "Title: "; std::getline(std::cin, title_raw);
  std::cout << "Content: "; std::getline(std::cin, content);

  engine_.CreateTask(core::Title(title_raw), content);
  std::cout << "Task created.\n";
}

void ConsoleApp::CreateNote() {
  std::string title_raw, content;
  std::cout << "Title: "; std::getline(std::cin, title_raw);
  std::cout << "Content: "; std::getline(std::cin, content);

  engine_.CreateNote(core::Title(title_raw), content);
  std::cout << "Note created.\n";
}

void ConsoleApp::DeleteEntity() {
  std::string id_raw;
  std::cout << "ID to remove: "; std::getline(std::cin, id_raw);
  engine_.RemoveEntity(core::ID(id_raw));
  std::cout << "Entity removal initiated.\n";
}

void ConsoleApp::MoveEntity() {
  std::string child_id, parent_id;
  std::cout << "Entity ID: "; std::getline(std::cin, child_id);
  std::cout << "New Parent ID (leave empty for root): "; std::getline(std::cin, parent_id);

  std::optional<core::ID> p_id;
  if (!parent_id.empty()) {
    p_id = core::ID(parent_id);
  }
  
  engine_.MoveEntity(core::ID(child_id), p_id);
  std::cout << "Entity moved.\n";
}

void ConsoleApp::Undo() {
  if (engine_.CanUndo()) {
    engine_.Undo();
    std::cout << "Undo successful.\n";
  } else {
    std::cout << "Nothing to undo.\n";
  }
}

void ConsoleApp::Redo() {
  if (engine_.CanRedo()) {
    engine_.Redo();
    std::cout << "Redo successful.\n";
  } else {
    std::cout << "Nothing to redo.\n";
  }
}

}  // namespace cli
