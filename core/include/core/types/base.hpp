#pragma once

#include <string>
#include <utility>

namespace core {

class StringWrapper {
 public:
  virtual ~StringWrapper() = default;
  const std::string& Str() const { return value_; }

 protected:
  explicit StringWrapper(std::string value) : value_(std::move(value)) {}
  std::string value_;
};

}  // namespace core
