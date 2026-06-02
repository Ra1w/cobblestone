#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "core/interfaces/storage.hpp"
#include "core/types/id.hpp"

namespace infra::storage {

class MarkdownStorage : public core::interfaces::IStorage {
 public:
  explicit MarkdownStorage(std::filesystem::path base_path);
  virtual ~MarkdownStorage() override;

  MarkdownStorage(const MarkdownStorage&) = delete;
  MarkdownStorage& operator=(const MarkdownStorage&) = delete;

  std::future<void> SaveAsync(const core::entities::Entity& entity) override;

  std::vector<core::interfaces::PersistenceEntry> LoadAll() override;

  void Remove(const core::ID& id) override;

 private:
  struct SaveTask {
    std::filesystem::path path;
    std::string content;
    std::promise<void> promise;
  };

  std::filesystem::path base_path_;
  
  std::queue<std::unique_ptr<SaveTask>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable cv_;
  std::atomic<bool> running_{true};
  std::thread worker_thread_;

  void WorkerLoop();
  std::filesystem::path GetPath(const core::ID& id) const;
};

}  // namespace infra::storage
