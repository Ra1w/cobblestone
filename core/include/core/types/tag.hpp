#pragma once

#include <vector>

#include "core/exceptions.hpp"
#include "core/types/base.hpp"

namespace core {

class Tag : public StringWrapper {
 public:
  explicit Tag(std::string value);

  bool operator==(const Tag& other) const;
  bool operator<(const Tag& other) const;
};

using TagList = std::vector<Tag>;

}  // namespace core
