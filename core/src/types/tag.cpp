#include "core/types/tag.hpp"

#include <algorithm>
#include <cctype>

namespace core {

Tag::Tag(std::string value) : StringWrapper(std::move(value)) {
  if (m_value_.empty()) throw ValidationError("Tag", "Value cannot be empty");

  std::transform(m_value_.begin(), m_value_.end(), m_value_.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

bool Tag::operator==(const Tag& other) const {
  return m_value_ == other.m_value_;
}
bool Tag::operator<(const Tag& other) const {
  return m_value_ < other.m_value_;
}

}  // namespace core
