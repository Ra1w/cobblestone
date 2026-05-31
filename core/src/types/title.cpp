#include "core/types/title.hpp"

namespace core {

Title::Title(std::string value) : StringWrapper(std::move(value)) {
  m_value_.erase(0, m_value_.find_first_not_of(" "));
  m_value_.erase(m_value_.find_last_not_of(" ") + 1);
  Validate(m_value_);
}

bool Title::operator==(const Title& other) const {
  return m_value_ == other.m_value_;
}

bool Title::operator!=(const Title& other) const { return !(*this == other); }

void Title::Validate(const std::string& val) {
  if (val.empty()) {
    throw ValidationError("Title", "Value cannot be empty");
  }
  if (val.length() > 255) {
    throw ValidationError("Title", "Exceeds 255 characters");
  }
}

}  // namespace core
