#include "core/types/timestamp.hpp"

#include <chrono>
#include <format>
#include <iomanip>
#include <sstream>

namespace core {

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
  return std::format("{:%Y-%m-%dT%H:%M:%SZ}",
                     std::chrono::floor<std::chrono::seconds>(m_tp_));
}

Timestamp Timestamp::FromIsoString(const std::string& iso_str) {
  std::tm tm = {};
  std::stringstream ss(iso_str);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

  if (ss.fail()) {
    throw ValidationError("Timestamp", "Invalid ISO 8601 format");
  }

  using namespace std::chrono;
  auto date = sys_days{year{tm.tm_year + 1900} / (tm.tm_mon + 1) / tm.tm_mday};
  auto time = hours{tm.tm_hour} + minutes{tm.tm_min} + seconds{tm.tm_sec};

  return Timestamp(system_clock::time_point(date + time));
}

}  // namespace core
