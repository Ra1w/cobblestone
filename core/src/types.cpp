#include "core/types.hpp"

#include <iomanip>
#include <random>
#include <sstream>

namespace core {

// ID

ID::ID(std::string value) : StringWrapper(std::move(value)) {
  Validate(m_value);
}

bool ID::operator==(const ID& other) const { return m_value == other.m_value; }
bool ID::operator!=(const ID& other) const { return !(*this == other); }
bool ID::operator<(const ID& other) const { return m_value < other.m_value; }

void ID::Validate(const std::string& value) {
  if (value.empty()) {
    throw ValidationError("ID", "Value cannot be empty");
  }
  const std::string forbidden = " /\\?%*:|\"<> \t\n\r";
  if (value.find_first_of(forbidden) != std::string::npos) {
    throw ValidationError("ID", "Contains forbidden characters");
  }
}

ID ID::Generate() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < 12; ++i) {
    ss << dis(gen);
  }
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  ss << "-" << std::hex << (now % 0xFFFF);
  return ID(ss.str());
}

// Title

Title::Title(std::string value) : StringWrapper(std::move(value)) {
  m_value.erase(0, m_value.find_first_not_of(" "));
  m_value.erase(m_value.find_last_not_of(" ") + 1);
  Validate(m_value);
}

bool Title::operator==(const Title& other) const {
  return m_value == other.m_value;
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

namespace std {

size_t hash<core::ID>::operator()(const core::ID& id) const {
  return std::hash<std::string>{}(id.str());
}

}  // namespace std