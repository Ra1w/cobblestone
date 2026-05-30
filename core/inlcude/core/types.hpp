#pragma once

#include <chrono>
#include <functional>
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
  explicit ID(std::string value);
  static ID Generate();

  bool operator==(const ID& other) const;
  bool operator!=(const ID& other) const;
  bool operator<(const ID& other) const;

 private:
  void Validate(const std::string& value);
};

class Title : public StringWrapper {
 public:
  explicit Title(std::string value);

  bool operator==(const Title& other) const;
  bool operator!=(const Title& other) const;

 private:
  void Validate(const std::string& value);
};

}  // namespace core

namespace std {

template <>
struct hash<core::ID> {
  size_t operator()(const core::ID& id) const;
};

}  // namespace std