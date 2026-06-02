#include "core/types/title.hpp"

namespace core {

Title::Title(std::string value) : StringWrapper(std::move(value)) {
  size_t first = value_.find_first_not_of(" ");
  if (first != std::string::npos) {
    value_.erase(0, first);
    size_t last = value_.find_last_not_of(" ");
    if (last != std::string::npos) {
      value_.erase(last + 1);
    }
  } else {
    value_.clear();
  }
  Validate(value_);
}

bool Title::operator==(const Title& other) const {
  return value_ == other.value_;
}

bool Title::operator!=(const Title& other) const { return !(*this == other); }

void Title::Validate(const std::string& val) {
  if (val.empty()) {
    throw ValidationError("Title", "Value cannot be empty");
  }
  if (val.length() > 255) {
    throw ValidationError("Title", "Exceeds 255 characters");
  }
  if (val.find_first_of("\n\r") != std::string::npos) {
    throw ValidationError("Title", "Contains newline characters");
  }
}

}  // namespace core
