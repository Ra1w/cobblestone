#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "core/exceptions.hpp"

namespace core {

class StringWrapper {
 public:
  virtual ~StringWrapper() = default;
  const std::string& str() const { return m_value; }

 protected:
  explicit StringWrapper(std::string value) : m_value(std::move(value)) {}
  std::string m_value;
};

class ID : public StringWrapper {
 public:
  explicit ID(std::string value) : StringWrapper(std::move(value)) {
    Validate(m_value);
  }

  static ID Generate();

  bool ID::operator==(const ID& other) const {
    return m_value == other.m_value;
  }
  bool ID::operator!=(const ID& other) const { return !(*this == other); }
  bool ID::operator<(const ID& other) const { return m_value < other.m_value; }

 private:
  void Validate(const std::string& value);
};

}  // namespace core
