#include <iostream>
#include <memory>

#include "app/life_engine.hpp"
#include "cli/console_app.hpp"
#include "infra/storage/markdown_storage.hpp"

int main() {
  try {
    auto storage = std::make_unique<infra::storage::MarkdownStorage>("./notes");

    app::LifeEngine engine(std::move(storage));

    cli::ConsoleApp app(engine);
    app.Run();

  } catch (const std::exception& e) {
    std::cerr << "\n[FATAL SYSTEM ERROR] " << e.what() << "\n";
    std::cerr << "Application terminated abnormally.\n";
    return 1;
  }

  return 0;
}