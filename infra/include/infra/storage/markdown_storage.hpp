#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <vector>

#include "core/interfaces/storage.hpp"
#include "core/types/id.hpp"

namespace infra::storage {

class MarkdownStorage : public core::interfaces::IStorage {
 public:
  explicit MarkdownStorage(std::filesystem::path base_path);

  virtual ~MarkdownStorage() override = default;

  std::future<void> SaveAsync(const core::entities::Entity& entity) override;

  std::vector<core::interfaces::PersistenceEntry> LoadAll() override;

  void Remove(const core::ID& id) override;

 private:
  std::filesystem::path base_path_;

  std::filesystem::path GetPath(const core::ID& id) const;
};

}  // namespace infra::storage
