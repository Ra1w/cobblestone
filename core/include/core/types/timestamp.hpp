#pragma once

#include <chrono>
#include <string>

#include "core/exceptions.hpp"

namespace core {

class Timestamp {
 public:
  Timestamp();
  explicit Timestamp(std::chrono::system_clock::time_point tp);

  static Timestamp Now();

  std::string ToIsoString() const;
  static Timestamp FromIsoString(const std::string& iso_str);

  bool operator==(const Timestamp& other) const;
  bool operator!=(const Timestamp& other) const;
  bool operator<(const Timestamp& other) const;
  bool operator>(const Timestamp& other) const;

  std::chrono::system_clock::time_point Raw() const { return tp_; }

 private:
  std::chrono::system_clock::time_point tp_;
};

}  // namespace core
