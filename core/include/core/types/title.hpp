#pragma once

#include "core/exceptions.hpp"
#include "core/types/base.hpp"

namespace core {

class Title : public StringWrapper {
 public:
  explicit Title(std::string value);

  bool operator==(const Title& other) const;
  bool operator!=(const Title& other) const;

 private:
  void Validate(const std::string& value);
};

}  // namespace core
