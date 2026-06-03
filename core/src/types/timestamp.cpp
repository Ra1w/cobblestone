#include "core/types/timestamp.hpp"

#include <chrono>
#include <format>
#include <sstream>

namespace core {

Timestamp::Timestamp() : tp_(std::chrono::system_clock::now()) {}
Timestamp::Timestamp(std::chrono::system_clock::time_point tp) : tp_(tp) {}
Timestamp Timestamp::Now() { return Timestamp(); }

bool Timestamp::operator==(const Timestamp& other) const {
  return tp_ == other.tp_;
}
bool Timestamp::operator!=(const Timestamp& other) const {
  return !(*this == other);
}
bool Timestamp::operator<(const Timestamp& other) const {
  return tp_ < other.tp_;
}
bool Timestamp::operator>(const Timestamp& other) const {
  return tp_ > other.tp_;
}

std::string Timestamp::ToIsoString() const {
  return std::format("{:%Y-%m-%dT%H:%M:%SZ}",
                     std::chrono::time_point_cast<std::chrono::seconds>(tp_));
}

Timestamp Timestamp::FromIsoString(const std::string& iso_str) {
  std::chrono::sys_seconds parsed_tp;
  std::istringstream in{iso_str};
  
  in >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", parsed_tp);

  if (in.fail()) {
    throw ValidationError("Timestamp", "Invalid ISO 8601 format");
  }

  return Timestamp(parsed_tp);
}

}  // namespace core
