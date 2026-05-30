#pragma once

#include <stdexcept>
#include <string>

namespace core {

class CoreException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class ValidationError : public CoreException {
 public:
  ValidationError(const std::string& field, const std::string& message)
      : CoreException("Validation failed for [" + field + "]: " + message)
      , m_field(field) {}

  const std::string& GetField() const { return m_field; }

 private:
  std::string m_field;
};

class LogicError : public CoreException {
 public:
  using CoreException::CoreException;
};

class NotFoundError : public CoreException {
 public:
  using CoreException::CoreException;
};

}  // namespace core