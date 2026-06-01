#include "infra/storage/markdown_storage.hpp"

#include <fstream>
#include <future>
#include <generator>
#include <iostream>
#include <sstream>

#include "core/exceptions.hpp"
#include "infra/serialization/markdown_serializer.hpp"

namespace infra::storage {

namespace ct = core;
namespace ci = core::interfaces;
namespace is = infra::serialization;

static std::generator<std::filesystem::path> EnumerateMarkdownFiles(
    std::filesystem::path path) {
  if (!std::filesystem::exists(path)) {
    co_return;
  }

  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".md") {
      co_yield entry.path();
    }
  }
}

MarkdownStorage::MarkdownStorage(std::filesystem::path base_path)
    : base_path_(std::move(base_path)) {
  if (!std::filesystem::exists(base_path_)) {
    std::filesystem::create_directories(base_path_);
  }
}

std::future<void> MarkdownStorage::SaveAsync(
    const ct::entities::Entity& entity) {
  std::string content = is::MarkdownSerializer::Serialize(entity);
  std::filesystem::path path = GetPath(entity.GetId());

  return std::async(std::launch::async, [path, content = std::move(content)]() {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
      throw ct::SystemError("MarkdownStorage: Cannot open file for writing: " +
                            path.string());
    }
    file << content;
  });
}

std::vector<ci::PersistenceEntry> MarkdownStorage::LoadAll() {
  std::vector<ci::PersistenceEntry> entries;

  for (auto&& path : EnumerateMarkdownFiles(base_path_)) {
    std::ifstream file(path);
    if (!file.is_open()) {
      continue;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    try {
      entries.push_back(is::MarkdownSerializer::Deserialize(buffer.str()));
    } catch (const ct::CoreException& e) {
      std::cerr << "[Warning] MarkdownStorage: Skipping corrupted file " << path
                << ": " << e.what() << "\n";
    }
  }
  return entries;
}

void MarkdownStorage::Remove(const ct::ID& id) {
  std::filesystem::path path = GetPath(id);
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
}

std::filesystem::path MarkdownStorage::GetPath(const ct::ID& id) const {
  return base_path_ / (id.Str() + ".md");
}

}  // namespace infra::storage
