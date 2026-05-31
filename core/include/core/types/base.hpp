#pragma once

#include <string>
#include <utility>

namespace core {

class StringWrapper {
 public:
  virtual ~StringWrapper() = default;
  const std::string& Str() const { return m_value_; }

 protected:
  explicit StringWrapper(std::string value) : m_value_(std::move(value)) {}
  std::string m_value_;
};

}  // namespace core
