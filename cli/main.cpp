#include <memory>
#include "app/life_engine.hpp"
#include "infra/storage/markdown_storage.hpp"
#include "cli/console_app.hpp"

int main() {
  auto storage = std::make_unique<infra::storage::MarkdownStorage>("./notes");

  app::LifeEngine engine(std::move(storage));

  cli::ConsoleApp app(engine);
  app.Run();

  return 0;
}