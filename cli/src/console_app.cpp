#include "cli/console_app.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <optional>
#include <print>
#include <ranges>
#include <string>

#include "core/entities/task.hpp"
#include "core/exceptions.hpp"

namespace cli {

ConsoleApp::ConsoleApp(app::LifeEngine& engine) : engine_(engine) {}

void ConsoleApp::Run() {
  try {
    engine_.Load();
    std::println("Life Gamificator 2026: Workspace loaded successfully.");
  } catch (const core::CoreException& e) {
    std::println("Warning during load: {}", e.what());
  }

  std::println("Welcome! Type 'help' to see available commands.");

  std::string line;
  while (running_) {
    std::print("\n> ");
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
  std::string command = input;
  
  size_t first = command.find_first_not_of(" \t\r\n");
  if (first != std::string::npos) {
    command = command.substr(first);
  } else {
    return;
  }
  
  size_t last = command.find_last_not_of(" \t\r\n");
  if (last != std::string::npos) {
    command = command.substr(0, last + 1);
  }

  try {
    if (command == "exit" || command == "quit") {
      std::println("Saving changes and exiting...");
      engine_.SaveAll();
      engine_.WaitAllSaves();
      running_ = false;
    } else if (command == "help") {
      ShowHelp();
    } else if (command == "ls") {
      ListRoot();
    } else if (command == "tasks") {
      ListTasks();
    } else if (command == "view") {
      ViewEntity();
    } else if (command == "add-t") {
      CreateTask();
    } else if (command == "add-n") {
      CreateNote();
    } else if (command == "rm") {
      DeleteEntity();
    } else if (command == "mv") {
      MoveEntity();
    } else if (command == "edit") {
      EditContent();
    } else if (command == "append") {
      AppendContent();
    } else if (command == "rename") {
      RenameEntity();
    } else if (command == "status") {
      SetStatus();
    } else if (command == "tag-add") {
      AddTag();
    } else if (command == "tag-rm") {
      RemoveTag();
    } else if (command == "find-tag") {
      FindByTag();
    } else if (command == "search") {
      SearchText();
    } else if (command == "undo") {
      Undo();
    } else if (command == "redo") {
      Redo();
    } else if (command == "save") {
      engine_.SaveAll();
      std::println("All data has been queued for saving.");
    } else {
      std::println("Unknown command. Type 'help' for a list of commands.");
    }
  } catch (const core::ValidationError& e) {
    std::println("Input Validation Error: {}", e.what());
  } catch (const core::NotFoundError& e) {
    std::println("Not Found: {}", e.what());
  } catch (const core::PersistenceError& e) {
    std::println("Data Storage Error: {}", e.what());
  } catch (const core::CoreException& e) {
    std::println("Error: {}", e.what());
  } catch (const std::logic_error& e) {
    std::println(stderr, "CRITICAL SOFTWARE BUG: {}", e.what());
    std::println(stderr,
                 "The application state may be corrupted. Please report this.");
  } catch (const std::exception& e) {
    std::println(stderr, "Unknown System Error: {}", e.what());
  }
}

void ConsoleApp::ShowHelp() {
  std::println(
      "Navigation & View:\n"
      "  ls       - Show hierarchy tree (sorted by new)\n"
      "  tasks    - List all tasks (can filter by status)\n"
      "  view     - Full view of an entity (ID required)\n"
      "Creation & Modification:\n"
      "  add-t    - Create new Task\n"
      "  add-n    - Create new Note\n"
      "  edit     - Overwrite entity content\n"
      "  append   - Add text to the end of entity content\n"
      "  rename   - Change entity title\n"
      "  status   - Change Task status (todo/inprogress/done)\n"
      "  tag-add  - Add tag to entity\n"
      "  tag-rm   - Remove tag from entity\n"
      "  mv       - Move entity to another parent\n"
      "  rm       - Delete entity\n"
      "Search & System:\n"
      "  search   - Full-text search in title and content\n"
      "  find-tag - Search entities by tag\n"
      "  undo     - Rollback last action\n"
      "  redo     - Repeat last undone action\n"
      "  save     - Force save to disk\n"
      "  exit     - Quit application");
}

void ConsoleApp::ListRoot() {
  auto roots = engine_.GetRootEntities();
  if (roots.empty()) {
    std::println("The knowledge base is currently empty.");
    return;
  }

  std::println("\nKnowledge Tree:");
  for (const auto* root : roots) {
    PrintTree(*root, 0);
  }
}

void ConsoleApp::PrintTree(const core::entities::Entity& entity, int depth) {
  std::string indent(depth * 3, ' ');
  std::string type_label = "[N]";
  if (entity.GetType() == core::EntityType::Task) {
    type_label = "[T]";
  }

  std::string status_info = "";
  if (entity.GetType() == core::EntityType::Task) {
    auto status = static_cast<const core::entities::Task&>(entity).GetStatus();
    if (status == core::TaskStatus::Done) {
      status_info = " (DONE)";
    } else if (status == core::TaskStatus::InProgress) {
      status_info = " (...)";
    } else {
      status_info = " (TODO)";
    }
  }

  std::println("{}|-- {} {}{} (ID: {})", indent, type_label,
               entity.GetMetadata().title.Str(), status_info,
               entity.GetId().Str());

  for (const auto& child : entity.GetChildren()) {
    PrintTree(*child, depth + 1);
  }
}

void ConsoleApp::ViewEntity() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  core::ID id = engine_.ResolveId(input);
  auto* entity = engine_.GetEntity(id);

  if (!entity) {
    std::println("Entity not found.");
    return;
  }

  const auto& meta = entity->GetMetadata();
  std::println("\n==================================================");
  std::println("TITLE:   {}", meta.title.Str());
  std::println("ID:      {}", meta.id.Str());

  std::string type_str = "Note";
  if (entity->GetType() == core::EntityType::Task) {
    type_str = "Task";
  }
  std::println("TYPE:    {}", type_str);

  if (entity->GetType() == core::EntityType::Task) {
    auto status = static_cast<core::entities::Task*>(entity)->GetStatus();
    std::string s_str = "Todo";
    if (status == core::TaskStatus::Done) {
      s_str = "Done";
    } else if (status == core::TaskStatus::InProgress) {
      s_str = "In Progress";
    }
    std::println("STATUS:  {}", s_str);
  }

  std::string tags_str;
  for (const auto& t : meta.tags) {
    tags_str += "[" + t.Str() + "] ";
  }

  if (tags_str.empty()) {
    tags_str = "None";
  }

  std::println("TAGS:    {}", tags_str);
  std::println("CREATED: {}", meta.created_at.ToIsoString());
  std::println("UPDATED: {}", meta.updated_at.ToIsoString());
  std::println("--------------------------------------------------");
  std::println("CONTENT:\n{}", entity->GetContent());
  std::println("==================================================");
}

void ConsoleApp::CreateTask() {
  std::print("Task Title: ");
  std::string title_raw;
  std::getline(std::cin, title_raw);

  std::print("Content: ");
  std::string content;
  std::getline(std::cin, content);

  engine_.CreateTask(core::Title(title_raw), content);
  std::println("Task created successfully.");
}

void ConsoleApp::CreateNote() {
  std::print("Note Title: ");
  std::string title_raw;
  std::getline(std::cin, title_raw);

  std::print("Content: ");
  std::string content;
  std::getline(std::cin, content);

  engine_.CreateNote(core::Title(title_raw), content);
  std::println("Note created successfully.");
}

void ConsoleApp::DeleteEntity() {
  std::print("Enter ID to delete: ");
  std::string input;
  std::getline(std::cin, input);

  core::ID id = engine_.ResolveId(input);
  engine_.RemoveEntity(id);
  std::println("Entity and its file marked for deletion.");
}

void ConsoleApp::MoveEntity() {
  std::print("Enter Entity ID to move: ");
  std::string child_input;
  std::getline(std::cin, child_input);

  std::print("Enter New Parent ID (or leave empty for root): ");
  std::string parent_input;
  std::getline(std::cin, parent_input);

  core::ID child_id = engine_.ResolveId(child_input);

  std::optional<core::ID> p_id;
  if (!parent_input.empty()) {
    p_id = engine_.ResolveId(parent_input);
  }

  engine_.MoveEntity(child_id, p_id);
  std::println("Entity moved successfully.");
}

void ConsoleApp::EditContent() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("Enter New Content: ");
  std::string new_content;
  std::getline(std::cin, new_content);

  core::ID id = engine_.ResolveId(input);
  engine_.EditEntity(id, [new_content](core::entities::Entity& e) {
    e.SetContent(new_content);
  });
  std::println("Content updated.");
}

void ConsoleApp::AppendContent() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("Enter Text to Append: ");
  std::string append_text;
  std::getline(std::cin, append_text);

  core::ID id = engine_.ResolveId(input);
  engine_.EditEntity(id, [append_text](core::entities::Entity& e) {
    std::string current = e.GetContent();
    if (!current.empty() && current.back() != '\n') {
      current += "\n";
    }
    current += append_text;
    e.SetContent(current);
  });
  std::println("Content appended.");
}

void ConsoleApp::RenameEntity() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("Enter New Title: ");
  std::string new_title;
  std::getline(std::cin, new_title);

  core::ID id = engine_.ResolveId(input);
  engine_.EditEntity(id, [new_title](core::entities::Entity& e) {
    e.SetTitle(core::Title(new_title));
  });
  std::println("Entity renamed.");
}

void ConsoleApp::SetStatus() {
  std::print("Enter Task ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("New Status (todo / inprogress / done): ");
  std::string status_str;
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
  std::println("Status updated.");
}

void ConsoleApp::Undo() {
  if (engine_.CanUndo()) {
    engine_.Undo();
    std::println("Undo successful.");
  } else {
    std::println("History is empty. Nothing to undo.");
  }
}

void ConsoleApp::Redo() {
  if (engine_.CanRedo()) {
    engine_.Redo();
    std::println("Redo successful.");
  } else {
    std::println("Nothing to redo.");
  }
}

core::TaskStatus ConsoleApp::ParseStatus(const std::string& status_str) {
  std::string s = status_str;
  std::ranges::transform(s, s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (s == "done") {
    return core::TaskStatus::Done;
  }

  if (s == "inprogress") {
    return core::TaskStatus::InProgress;
  }

  if (s == "todo") {
    return core::TaskStatus::Todo;
  }

  throw core::ValidationError(
      "TaskStatus", "Unknown status. Use 'todo', 'inprogress', or 'done'.");
}

void ConsoleApp::AddTag() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("Enter Tag: ");
  std::string tag_str;
  std::getline(std::cin, tag_str);

  core::ID id = engine_.ResolveId(input);
  core::Tag new_tag(tag_str);

  engine_.EditEntity(id, [new_tag](core::entities::Entity& e) {
    auto tags = e.GetMetadata().tags;

    if (!std::ranges::contains(tags, new_tag)) {
      tags.push_back(new_tag);
      e.SetTags(tags);
    }
  });
  std::println("Tag added.");
}

void ConsoleApp::RemoveTag() {
  std::print("Enter Entity ID: ");
  std::string input;
  std::getline(std::cin, input);

  std::print("Enter Tag: ");
  std::string tag_str;
  std::getline(std::cin, tag_str);

  core::ID id = engine_.ResolveId(input);
  core::Tag target_tag(tag_str);

  engine_.EditEntity(id, [target_tag](core::entities::Entity& e) {
    auto tags = e.GetMetadata().tags;

    if (std::erase(tags, target_tag) > 0) {
      e.SetTags(tags);
    }
  });
  std::println("Tag removed (if it existed).");
}

void ConsoleApp::FindByTag() {
  std::print("Enter Tag to search: ");
  std::string tag_str;
  std::getline(std::cin, tag_str);

  core::Tag search_tag(tag_str);
  auto results = engine_.FindByTag(search_tag);

  if (results.empty()) {
    std::println("No entities found with tag [{}].", tag_str);
    return;
  }

  std::println("\nSearch Results:");
  for (const auto* e : results) {
    std::string type_label = "[N]";
    if (e->GetType() == core::EntityType::Task) {
      type_label = "[T]";
    }
    std::println("{} {} (ID: {})", type_label, e->GetMetadata().title.Str(),
                 e->GetId().Str());
  }
}

void ConsoleApp::SearchText() {
  std::print("Enter search query: ");
  std::string query;
  std::getline(std::cin, query);

  if (query.empty()) {
    std::println("Query cannot be empty.");
    return;
  }

  auto results = engine_.Search(query);

  if (results.empty()) {
    std::println("No matches found for [{}].", query);
    return;
  }

  std::println("\nSearch Results:");
  for (const auto* e : results) {
    std::string type_label = "[N]";
    if (e->GetType() == core::EntityType::Task) {
      type_label = "[T]";
    }

    std::string snippet = e->GetContent();
    if (snippet.length() > 50) {
      snippet = snippet.substr(0, 47) + "...";
    }

    std::ranges::replace(snippet, '\n', ' ');

    std::println("{} {} (ID: {})\n      Content: {}", type_label,
                 e->GetMetadata().title.Str(), e->GetId().Str(), snippet);
  }
}

void ConsoleApp::ListTasks() {
  std::print(
      "Enter status filter (todo/inprogress/done) or leave empty for all: ");
  std::string status_input;
  std::getline(std::cin, status_input);

  std::optional<core::TaskStatus> filter;
  if (!status_input.empty()) {
    filter = ParseStatus(status_input);
  }

  auto tasks = engine_.GetTasks(filter);
  if (tasks.empty()) {
    std::println("No tasks found matching criteria.");
    return;
  }

  std::println("\nTasks List:");
  for (const auto* t : tasks) {
    std::string status_info;
    auto status = t->GetStatus();

    if (status == core::TaskStatus::Done) {
      status_info = "[DONE]";
    } else if (status == core::TaskStatus::InProgress) {
      status_info = "[...] ";
    } else {
      status_info = "[TODO]";
    }

    std::println("{} {} (ID: {})", status_info, t->GetMetadata().title.Str(),
                 t->GetId().Str());
  }
}

}  // namespace cli
