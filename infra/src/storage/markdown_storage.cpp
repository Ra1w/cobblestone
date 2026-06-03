#include "infra/storage/markdown_storage.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "core/exceptions.hpp"
#include "infra/serialization/markdown_serializer.hpp"

namespace infra::storage {

MarkdownStorage::MarkdownStorage(std::filesystem::path base_path)
    : base_path_(std::move(base_path)) {
  if (!std::filesystem::exists(base_path_)) {
    std::filesystem::create_directories(base_path_);
  }
  worker_thread_ = std::thread(&MarkdownStorage::WorkerLoop, this);
}

MarkdownStorage::~MarkdownStorage() {
  {
    std::lock_guard lock(queue_mutex_);
    running_ = false;
  }
  cv_.notify_all();
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

std::future<void> MarkdownStorage::SaveAsync(
    const core::entities::Entity& entity) {
  auto task = std::make_unique<StorageTask>();
  task->type = StorageTask::Type::Save;
  task->path = GetPath(entity.GetId());
  task->content = infra::serialization::MarkdownSerializer::Serialize(entity);

  auto future = task->promise.get_future();

  {
    std::lock_guard lock(queue_mutex_);
    if (!running_) {
      task->promise.set_exception(std::make_exception_ptr(
          core::SystemError("Storage worker is shutting down")));
      return future;
    }
    tasks_.push(std::move(task));
  }
  cv_.notify_one();

  return future;
}

void MarkdownStorage::WorkerLoop() {
  while (true) {
    std::unique_ptr<StorageTask> task;
    {
      std::unique_lock lock(queue_mutex_);
      cv_.wait(lock, [this] { return !tasks_.empty() || !running_; });

      if (!running_ && tasks_.empty()) {
        break;
      }

      if (!tasks_.empty()) {
        task = std::move(tasks_.front());
        tasks_.pop();
      }
    }

    if (task) {
      try {
        if (task->type == StorageTask::Type::Save) {
          std::filesystem::path tmp_path = task->path;
          tmp_path += ".tmp";

          std::ofstream file(tmp_path, std::ios::trunc);
          if (!file.is_open()) {
            throw core::SystemError("Cannot open file: " + tmp_path.string());
          }

          file << task->content;
          file.flush();

          if (!file) {
            file.close();
            throw core::SystemError("Failed to write data to file: " +
                                    tmp_path.string());
          }
          file.close();

          std::filesystem::rename(tmp_path, task->path);
        } else if (task->type == StorageTask::Type::Remove) {
          if (std::filesystem::exists(task->path)) {
            std::filesystem::remove(task->path);
          }
        }
        task->promise.set_value();
      } catch (const std::exception& e) {
        std::cerr << "Storage worker exception: " << e.what() << "\n";
        task->promise.set_exception(std::current_exception());
      } catch (...) {
        task->promise.set_exception(std::current_exception());
      }
    }
  }
}

std::vector<core::interfaces::PersistenceEntry> MarkdownStorage::LoadAll() {
  std::vector<core::interfaces::PersistenceEntry> entries;

  if (!std::filesystem::exists(base_path_) ||
      !std::filesystem::is_directory(base_path_)) {
    return entries;
  }

  try {
    for (const auto& entry : std::filesystem::directory_iterator(base_path_)) {
      if (entry.is_regular_file() && entry.path().extension() == ".md") {
        std::ifstream file(entry.path());
        if (file.is_open()) {
          std::stringstream buffer;
          buffer << file.rdbuf();
          try {
            entries.push_back(
                infra::serialization::MarkdownSerializer::Deserialize(
                    buffer.str()));
          } catch (const core::CoreException& e) {
            std::cerr << "[Critical] Failed to load " << entry.path() << ": "
                      << e.what() << "\n";
          }
        }
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "[Critical] Filesystem error while reading directory: "
              << e.what() << "\n";
  }

  return entries;
}

void MarkdownStorage::Remove(const core::ID& id) {
  auto task = std::make_unique<StorageTask>();
  task->type = StorageTask::Type::Remove;
  task->path = GetPath(id);

  {
    std::lock_guard lock(queue_mutex_);
    if (!running_) {
      return;
    }
    tasks_.push(std::move(task));
  }
  cv_.notify_one();
}

std::filesystem::path MarkdownStorage::GetPath(const core::ID& id) const {
  return base_path_ / (id.Str() + ".md");
}

}  // namespace infra::storage
