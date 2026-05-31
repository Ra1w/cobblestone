#pragma once

#include <functional>
#include <string>

#include "core/exceptions.hpp"
#include "core/types/base.hpp"

namespace core {

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

}  // namespace core

namespace std {

template <>
struct hash<core::ID> {
  size_t operator()(const core::ID& id) const;
};

}  // namespace std
