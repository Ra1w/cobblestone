#include "core/types/timestamp.hpp"

#include <chrono>
#include <cstdio>
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
  int year, month, day, hour, minute, second;

  if (std::sscanf(iso_str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month,
                  &day, &hour, &minute, &second) != 6) {
    throw ValidationError("Timestamp", "Invalid ISO 8601 format");
  }

  std::chrono::year_month_day ymd{std::chrono::year(year),
                                  std::chrono::month(month),
                                  std::chrono::day(day)};

  if (!ymd.ok() || hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
      second < 0 || second > 60) {
    throw ValidationError("Timestamp", "Invalid date or time values");
  }

  auto tp = std::chrono::sys_days{ymd} + std::chrono::hours{hour} +
            std::chrono::minutes{minute} + std::chrono::seconds{second};

  return Timestamp(tp);
}

}  // namespace core
