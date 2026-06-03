#pragma once

#include <string>
#include <utility>

namespace core {

class StringWrapper {
 public:
  virtual ~StringWrapper() = default;

  StringWrapper(const StringWrapper&) = default;
  StringWrapper& operator=(const StringWrapper&) = default;
  StringWrapper(StringWrapper&&) noexcept = default;
  StringWrapper& operator=(StringWrapper&&) noexcept = default;

  const std::string& Str() const { return value_; }

 protected:
  explicit StringWrapper(std::string value) : value_(std::move(value)) {}
  std::string value_;
};

enum class EntityType { Base, Task, Note };

enum class TaskStatus { Todo, InProgress, Done };

}  // namespace core
