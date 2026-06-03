#pragma once

#include <gmock/gmock.h>

#include <future>
#include <vector>

#include "core/interfaces/persistence_entry.hpp"
#include "core/interfaces/storage.hpp"

namespace tests::mocks {

class MockStorage : public core::interfaces::IStorage {
 public:
  MOCK_METHOD(std::future<void>, SaveAsync, (const core::entities::Entity&),
              (override));
  MOCK_METHOD(std::vector<core::interfaces::PersistenceEntry>, LoadAll, (),
              (override));
  MOCK_METHOD(void, Remove, (const core::ID&), (override));
};

inline std::future<void> MakeReadyFuture() {
  std::promise<void> p;
  p.set_value();
  return p.get_future();
}

}  // namespace tests::mocks
