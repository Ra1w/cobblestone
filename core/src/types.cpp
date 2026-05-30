#include "core/types.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

namespace core {

// ID

ID::ID(std::string value) : StringWrapper(std::move(value)) {
  Validate(m_value_);
}

bool ID::operator==(const ID& other) const {
  return m_value_ == other.m_value_;
}
bool ID::operator!=(const ID& other) const { return !(*this == other); }
bool ID::operator<(const ID& other) const { return m_value_ < other.m_value_; }

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

// Tag

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

// Timestamp

Timestamp::Timestamp() : m_tp_(std::chrono::system_clock::now()) {}

Timestamp::Timestamp(std::chrono::system_clock::time_point tp) : m_tp_(tp) {}

Timestamp Timestamp::Now() { return Timestamp(); }

bool Timestamp::operator==(const Timestamp& other) const {
  return m_tp_ == other.m_tp_;
}

bool Timestamp::operator!=(const Timestamp& other) const {
  return !(*this == other);
}

bool Timestamp::operator<(const Timestamp& other) const {
  return m_tp_ < other.m_tp_;
}

bool Timestamp::operator>(const Timestamp& other) const {
  return m_tp_ > other.m_tp_;
}

std::string Timestamp::ToIsoString() const {
  std::time_t tt = std::chrono::system_clock::to_time_t(m_tp_);
  std::tm gmt{};

  gmtime_r(&tt, &gmt);

  std::stringstream ss;
  ss << std::put_time(&gmt, "%Y-%m-%dT%H:%M:%SZ");
  return ss.str();
}

Timestamp Timestamp::FromIsoString(const std::string& iso_str) {
  std::tm tm = {};
  std::stringstream ss(iso_str);

  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

  if (ss.fail()) {
    throw ValidationError("Timestamp", "Invalid ISO 8601 format: " + iso_str);
  }

  std::time_t tt = timegm(&tm);
  return Timestamp(std::chrono::system_clock::from_time_t(tt));
}

}  // namespace core

namespace std {

size_t hash<core::ID>::operator()(const core::ID& id) const {
  return std::hash<std::string>{}(id.Str());
}

}  // namespace std