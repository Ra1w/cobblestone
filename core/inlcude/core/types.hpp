#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "core/exceptions.hpp"

namespace core {

class StringWrapper {
 public:
  virtual ~StringWrapper() = default;
  const std::string& Str() const { return m_value_; }

 protected:
  explicit StringWrapper(std::string value) : m_value_(std::move(value)) {}
  std::string m_value_;
};

class ID : public StringWrapper {
 public:
  explicit ID(std::string value);
  static ID Generate();

  bool operator==(const ID& other) const;
  bool operator!=(const ID& other) const;
  bool operator<(const ID& other) const;

 private:
  void Validate(const std::string& value);
};

class Title : public StringWrapper {
 public:
  explicit Title(std::string value);

  bool operator==(const Title& other) const;
  bool operator!=(const Title& other) const;

 private:
  void Validate(const std::string& value);
};

class Tag : public StringWrapper {
 public:
  explicit Tag(std::string value);

  bool operator==(const Tag& other) const;
  bool operator<(const Tag& other) const;
};

using TagList = std::vector<Tag>;

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

  std::chrono::system_clock::time_point Raw() const { return m_tp_; }

 private:
  std::chrono::system_clock::time_point m_tp_;
};


}  // namespace core

namespace std {

template <>
struct hash<core::ID> {
  size_t operator()(const core::ID& id) const;
};

}  // namespace std