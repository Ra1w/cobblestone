#pragma once

#include <string>

#include "core/entities/entity.hpp"
#include "core/interfaces/persistence_entry.hpp"

namespace infra::serialization {

class MarkdownSerializer {
 public:
  static std::string Serialize(const core::entities::Entity& entity);

  static core::interfaces::PersistenceEntry Deserialize(
      const std::string& raw_content);

 private:
  static std::string ExtractYamlValue(const std::string& yaml,
                                      const std::string& key);
  static std::vector<core::Tag> ParseTags(const std::string& tags_str);
};

}  // namespace infra::serialization
