#include "cli/console_app.hpp"

#include <algorithm>
#include <format>
#include <iostream>
#include <sstream>
#include <optional>
#include <cctype>

#include "core/entities/task.hpp"
#include "core/exceptions.hpp"

namespace cli {

ConsoleApp::ConsoleApp(app::LifeEngine& engine) : engine_(engine) {}

void ConsoleApp::Run() {
  try {
    engine_.Load();
    std::cout << "Life Gamificator 2026: Workspace loaded successfully.\n";
  } catch (const core::CoreException& e) {
    std::cout << std::format("Warning during load: {}\n", e.what());
  }

  std::cout << "Welcome! Type 'help' to see available commands.\n";

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
      std::cout << "Saving changes and exiting...\n";
      engine_.SaveAll();
      engine_.WaitAllSaves();
      running_ = false;
    } else if (input == "help") {
      ShowHelp();
    } else if (input == "ls") {
      ListRoot();
    } else if (input == "view") {
      ViewEntity();
    } else if (input == "add-t") {
      CreateTask();
    } else if (input == "add-n") {
      CreateNote();
    } else if (input == "rm") {
      DeleteEntity();
    } else if (input == "mv") {
      MoveEntity();
    } else if (input == "edit") {
      EditContent();
    } else if (input == "rename") {
      RenameEntity();
    } else if (input == "status") {
      SetStatus();
    } else if (input == "undo") {
      Undo();
    } else if (input == "redo") {
      Redo();
    } else if (input == "save") {
      engine_.SaveAll();
      std::cout << "All data has been queued for saving.\n";
    } else {
      std::cout << "Unknown command. Type 'help' for a list of commands.\n";
    }
  } catch (const core::ValidationError& e) {
    std::cout << std::format("Input Validation Error: {}\n", e.what());
  } catch (const core::NotFoundError& e) {
    std::cout << std::format("Not Found: {}\n", e.what());
  } catch (const core::PersistenceError& e) {
    std::cout << std::format("Data Storage Error: {}\n", e.what());
  } catch (const core::CoreException& e) {
    std::cout << std::format("Error: {}\n", e.what());
  } catch (const std::logic_error& e) {
    std::cerr << std::format("CRITICAL SOFTWARE BUG: {}\n", e.what());
    std::cerr << "The application state may be corrupted. Please report this.\n";
  } catch (const std::exception& e) {
    std::cerr << std::format("Unknown System Error: {}\n", e.what());
  }
}

void ConsoleApp::ShowHelp() {
  std::cout << "Navigation & View:\n"
            << "  ls       - Show hierarchy tree\n"
            << "  view     - Full view of an entity (ID required)\n"
            << "Creation & Modification:\n"
            << "  add-t    - Create new Task\n"
            << "  add-n    - Create new Note\n"
            << "  edit     - Edit entity content\n"
            << "  rename   - Change entity title\n"
            << "  status   - Change Task status (Todo/InProgress/Done)\n"
            << "  mv       - Move entity to another parent\n"
            << "  rm       - Delete entity\n"
            << "History & System:\n"
            << "  undo     - Rollback last action\n"
            << "  redo     - Repeat last undone action\n"
            << "  save     - Force save to disk\n"
            << "  exit     - Quit application\n";
}

void ConsoleApp::ListRoot() {
  auto roots = engine_.GetRootEntities();
  if (roots.empty()) {
    std::cout << "The knowledge base is currently empty.\n";
    return;
  }

  std::cout << "\nKnowledge Tree:\n";
  for (const auto* root : roots) {
    PrintTree(*root, 0);
  }
}

void ConsoleApp::PrintTree(const core::entities::Entity& entity, int depth) {
  std::string indent(depth * 3, ' ');
  std::string type_label =
      (entity.GetType() == core::EntityType::Task) ? "[T]" : "[N]";

  std::string status_info = "";
  if (entity.GetType() == core::EntityType::Task) {
    auto status = static_cast<const core::entities::Task&>(entity).GetStatus();
    status_info = (status == core::TaskStatus::Done)         ? " (DONE)"
                  : (status == core::TaskStatus::InProgress) ? " (...)"
                                                             : " (TODO)";
  }

  std::cout << std::format("{}|-- {} {}{} (ID: {})\n", indent, type_label,
                           entity.GetMetadata().title.Str(), status_info,
                           entity.GetId().Str());

  for (const auto& child : entity.GetChildren()) {
    PrintTree(*child, depth + 1);
  }
}

void ConsoleApp::ViewEntity() {
  std::cout << "Enter Entity ID: ";
  std::string input;
  std::getline(std::cin, input);

  core::ID id = engine_.ResolveId(input);
  auto* entity = engine_.GetEntity(id);
  
  if (!entity) {
    std::cout << "Entity not found.\n";
    return;
  }

  const auto& meta = entity->GetMetadata();
  std::cout << "\n==================================================\n";
  std::cout << std::format("TITLE:   {}\n", meta.title.Str());
  std::cout << std::format("ID:      {}\n", meta.id.Str());
  std::cout << std::format(
      "TYPE:    {}\n",
      (entity->GetType() == core::EntityType::Task ? "Task" : "Note"));

  if (entity->GetType() == core::EntityType::Task) {
    auto status = static_cast<core::entities::Task*>(entity)->GetStatus();
    std::string s_str = (status == core::TaskStatus::Done) ? "Done"
                        : (status == core::TaskStatus::InProgress)
                            ? "In Progress"
                            : "Todo";
    std::cout << std::format("STATUS:  {}\n", s_str);
  }

  std::cout << std::format("CREATED: {}\n", meta.created_at.ToIsoString());
  std::cout << std::format("UPDATED: {}\n", meta.updated_at.ToIsoString());
  std::cout << "--------------------------------------------------\n";
  std::cout << "CONTENT:\n" << entity->GetContent() << "\n";
  std::cout << "==================================================\n";
}

void ConsoleApp::CreateTask() {
  std::string title_raw;
  std::string content;
  std::cout << "Task Title: ";
  std::getline(std::cin, title_raw);
  std::cout << "Content: ";
  std::getline(std::cin, content);

  engine_.CreateTask(core::Title(title_raw), content);
  std::cout << "Task created successfully.\n";
}

void ConsoleApp::CreateNote() {
  std::string title_raw;
  std::string content;
  std::cout << "Note Title: ";
  std::getline(std::cin, title_raw);
  std::cout << "Content: ";
  std::getline(std::cin, content);

  engine_.CreateNote(core::Title(title_raw), content);
  std::cout << "Note created successfully.\n";
}

void ConsoleApp::DeleteEntity() {
  std::cout << "Enter ID to delete: ";
  std::string input;
  std::getline(std::cin, input);

  core::ID id = engine_.ResolveId(input);
  engine_.RemoveEntity(id);
  std::cout << "Entity and its file marked for deletion.\n";
}

void ConsoleApp::MoveEntity() {
  std::string child_input;
  std::string parent_input;
  std::cout << "Enter Entity ID to move: ";
  std::getline(std::cin, child_input);
  std::cout << "Enter New Parent ID (or leave empty for root): ";
  std::getline(std::cin, parent_input);

  core::ID child_id = engine_.ResolveId(child_input);
  
  std::optional<core::ID> p_id;
  if (!parent_input.empty()) {
    p_id = engine_.ResolveId(parent_input);
  }

  engine_.MoveEntity(child_id, p_id);
  std::cout << "Entity moved successfully.\n";
}

void ConsoleApp::EditContent() {
  std::string input;
  std::string new_content;
  std::cout << "Enter Entity ID: ";
  std::getline(std::cin, input);
  std::cout << "Enter New Content: ";
  std::getline(std::cin, new_content);

  core::ID id = engine_.ResolveId(input);
  engine_.EditEntity(
      id,
      [new_content](core::entities::Entity& e) { e.SetContent(new_content); });
  std::cout << "Content updated.\n";
}

void ConsoleApp::RenameEntity() {
  std::string input;
  std::string new_title;
  std::cout << "Enter Entity ID: ";
  std::getline(std::cin, input);
  std::cout << "Enter New Title: ";
  std::getline(std::cin, new_title);

  core::ID id = engine_.ResolveId(input);
  engine_.EditEntity(id, [new_title](core::entities::Entity& e) {
    e.SetTitle(core::Title(new_title));
  });
  std::cout << "Entity renamed.\n";
}

void ConsoleApp::SetStatus() {
  std::string input;
  std::string status_str;
  std::cout << "Enter Task ID: ";
  std::getline(std::cin, input);
  std::cout << "New Status (todo / inprogress / done): ";
  std::getline(std::cin, status_str);

  core::ID id = engine_.ResolveId(input);
  core::TaskStatus status = ParseStatus(status_str);

  engine_.EditEntity(id, [status](core::entities::Entity& e) {
    if (e.GetType() == core::EntityType::Task) {
      static_cast<core::entities::Task&>(e).SetStatus(status);
    } else {
      throw core::CommandError("Target entity is not a Task.");
    }
  });
  std::cout << "Status updated.\n";
}

void ConsoleApp::Undo() {
  if (engine_.CanUndo()) {
    engine_.Undo();
    std::cout << "Undo successful.\n";
  } else {
    std::cout << "History is empty. Nothing to undo.\n";
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

core::TaskStatus ConsoleApp::ParseStatus(const std::string& status_str) {
  std::string s = status_str;
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);

  if (s == "done") return core::TaskStatus::Done;
  if (s == "inprogress") return core::TaskStatus::InProgress;
  if (s == "todo") return core::TaskStatus::Todo;

  throw core::ValidationError(
      "TaskStatus", "Unknown status. Use 'todo', 'inprogress', or 'done'.");
}

}  // namespace cli
