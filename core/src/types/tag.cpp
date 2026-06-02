#include "core/types/tag.hpp"

#include <algorithm>
#include <cctype>

namespace core {

Tag::Tag(std::string value) : StringWrapper(std::move(value)) {
  if (value_.empty()) {
    throw ValidationError("Tag", "Value cannot be empty");
  }

  std::transform(
      value_.begin(), value_.end(), value_.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      });
}

bool Tag::operator==(const Tag& other) const { return value_ == other.value_; }
bool Tag::operator<(const Tag& other) const { return value_ < other.value_; }

}  // namespace core
