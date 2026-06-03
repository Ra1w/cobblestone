#include "core/types/id.hpp"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

namespace core {

ID::ID(std::string value) : StringWrapper(std::move(value)) {
  Validate(value_);
}

bool ID::operator==(const ID& other) const { return value_ == other.value_; }
bool ID::operator!=(const ID& other) const { return !(*this == other); }
bool ID::operator<(const ID& other) const { return value_ < other.value_; }

void ID::Validate(const std::string& value) {
  if (value.empty()) {
    throw ValidationError("ID", "Identifier cannot be empty");
  }
  const std::string forbidden = " /\\?%*:|\"<> \t\n\r";
  if (value.find_first_of(forbidden) != std::string::npos) {
    throw ValidationError("ID",
                          "Identifier contains forbidden characters or spaces");
  }
}

ID ID::Generate() {
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_int_distribution<> dis(0, 15);
  static std::atomic<uint64_t> counter{0};

  std::stringstream ss;

  for (int i = 0; i < 12; ++i) {
    ss << std::hex << dis(gen);
  }

  auto now = std::chrono::system_clock::now().time_since_epoch().count();

  ss << "-" << std::hex << std::setw(4) << std::setfill('0') << (now % 0xFFFF);
  ss << "-" << std::hex << std::setw(4) << std::setfill('0')
     << (counter.fetch_add(1) % 0xFFFF);

  return ID(ss.str());
}

}  // namespace core

namespace std {

size_t hash<core::ID>::operator()(const core::ID& id) const {
  return std::hash<std::string>{}(id.Str());
}

}  // namespace std
