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
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);
  static std::atomic<uint64_t> counter{0};

  std::stringstream ss;
  ss << std::hex << std::setfill('0');

  for (int i = 0; i < 12; ++i) {
    ss << dis(gen);
  }

  auto now = std::chrono::system_clock::now().time_since_epoch().count();
  ss << "-" << std::hex << (now % 0xFFFF);
  ss << "-" << std::hex << (counter.fetch_add(1) % 0xFFFF);

  return ID(ss.str());
}

}  // namespace core
