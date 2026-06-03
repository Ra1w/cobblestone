#include "core/types/id.hpp"

#include <atomic>
#include <chrono>
#include <format>
#include <random>

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
  thread_local std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
  static std::atomic<uint64_t> counter{0};

  auto now = std::chrono::system_clock::now().time_since_epoch().count();
  uint64_t c = counter.fetch_add(1);

  std::string result = std::format("{:08x}{:04x}-{:04x}-{:04x}", dis(gen),
                                   dis(gen) & 0xFFFF, now % 0xFFFF, c % 0xFFFF);

  return ID(std::move(result));
}

}  // namespace core

namespace std {

size_t hash<core::ID>::operator()(const core::ID& id) const {
  return std::hash<std::string>{}(id.Str());
}

}  // namespace std