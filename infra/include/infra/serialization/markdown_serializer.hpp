#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "core/entities/entity.hpp"
#include "core/interfaces/persistence_entry.hpp"

namespace infra::serialization {

class MarkdownSerializer {
 public:
  static std::string Serialize(const core::entities::Entity& entity);

  static core::interfaces::PersistenceEntry Deserialize(
      const std::string& raw_content);

 private:
  static std::unordered_map<std::string, std::string> ParseYamlBlock(
      const std::string& yaml);

  static std::vector<core::Tag> ParseTags(const std::string& tags_str);
};

}  // namespace infra::serialization