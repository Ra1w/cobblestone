#include "infra/serialization/markdown_serializer.hpp"

#include <format>
#include <sstream>

#include "core/entities/note.hpp"
#include "core/entities/task.hpp"
#include "core/exceptions.hpp"

namespace infra::serialization {

namespace ce = core::entities;
namespace ct = core;
namespace ci = core::interfaces;

std::string MarkdownSerializer::Serialize(const ce::Entity& entity) {
  const ce::Metadata& meta = entity.GetMetadata();

  std::string type_str = "note";
  if (entity.GetType() == ct::EntityType::Task) {
    type_str = "task";
  }

  std::string extra_fields = "";
  if (entity.GetType() == ct::EntityType::Task) {
    const auto& task = static_cast<const ce::Task&>(entity);
    ct::TaskStatus s = task.GetStatus();

    std::string s_str = "todo";
    if (s == ct::TaskStatus::Done) {
      s_str = "done";
    } else if (s == ct::TaskStatus::InProgress) {
      s_str = "inprogress";
    }
    extra_fields = std::format("status: {}\n", s_str);
  }

  std::string tags_str = "[";
  for (size_t i = 0; i < meta.tags.size(); ++i) {
    tags_str += meta.tags[i].Str();
    if (i < meta.tags.size() - 1) {
      tags_str += ", ";
    }
  }
  tags_str += "]";

  std::string parent_val = "null";
  if (entity.GetParent() != nullptr) {
    parent_val = entity.GetParent()->GetId().Str();
  }

  return std::format(
      "---\n"
      "id: {}\n"
      "type: {}\n"
      "title: {}\n"
      "{}"
      "tags: {}\n"
      "created_at: {}\n"
      "updated_at: {}\n"
      "parent: {}\n"
      "---\n"
      "{}",
      meta.id.Str(), type_str, meta.title.Str(), extra_fields, tags_str,
      meta.created_at.ToIsoString(), meta.updated_at.ToIsoString(), parent_val,
      entity.GetContent());
}

ci::PersistenceEntry MarkdownSerializer::Deserialize(
    const std::string& raw_content) {
  const std::string start_sep = "---";
  const std::string end_sep = "\n---";

  size_t first_sep = raw_content.find(start_sep);

  if (first_sep != 0) {
    throw ct::PersistenceError(
        "MarkdownSerializer: Invalid file format (YAML header must be at the "
        "very beginning of the file)");
  }

  size_t second_sep = raw_content.find(end_sep, first_sep + start_sep.length());
  if (second_sep == std::string::npos) {
    throw ct::PersistenceError(
        "MarkdownSerializer: Invalid file format (YAML header not closed)");
  }

  std::string yaml =
      raw_content.substr(first_sep + start_sep.length(),
                         second_sep - (first_sep + start_sep.length()));

  size_t end_of_header_line =
      raw_content.find('\n', second_sep + end_sep.length());
  std::string content = "";
  if (end_of_header_line != std::string::npos) {
    content = raw_content.substr(end_of_header_line + 1);
  }

  auto yaml_map = ParseYamlBlock(yaml);

  if (!yaml_map.contains("id")) {
    throw ct::PersistenceError(
        "MarkdownSerializer: Missing mandatory field [id]");
  }
  if (!yaml_map.contains("type")) {
    throw ct::PersistenceError(
        "MarkdownSerializer: Missing mandatory field [type]");
  }

  ct::ID id(yaml_map.at("id"));
  ct::Title title(yaml_map.contains("title") ? yaml_map.at("title") : "");
  auto tags = ParseTags(yaml_map.contains("tags") ? yaml_map.at("tags") : "");
  auto created = ct::Timestamp::FromIsoString(
      yaml_map.contains("created_at") ? yaml_map.at("created_at") : "");
  auto updated = ct::Timestamp::FromIsoString(
      yaml_map.contains("updated_at") ? yaml_map.at("updated_at") : "");

  std::optional<ct::ID> parent_id;
  if (yaml_map.contains("parent") && yaml_map.at("parent") != "null") {
    parent_id = ct::ID(yaml_map.at("parent"));
  }

  ce::Metadata meta(id, title, tags);
  meta.created_at = created;
  meta.updated_at = updated;

  std::unique_ptr<ce::Entity> entity;
  if (yaml_map.at("type") == "task") {
    std::string s_str =
        yaml_map.contains("status") ? yaml_map.at("status") : "todo";
    ct::TaskStatus s = ct::TaskStatus::Todo;
    if (s_str == "done") {
      s = ct::TaskStatus::Done;
    } else if (s_str == "inprogress") {
      s = ct::TaskStatus::InProgress;
    }
    entity = std::make_unique<ce::Task>(meta, content, s);
  } else {
    entity = std::make_unique<ce::Note>(meta, content);
  }

  return {std::move(entity), parent_id};
}

std::unordered_map<std::string, std::string> MarkdownSerializer::ParseYamlBlock(
    const std::string& yaml) {
  std::unordered_map<std::string, std::string> result;
  std::stringstream ss(yaml);
  std::string line;

  while (std::getline(ss, line)) {
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      std::string key = line.substr(0, colon_pos);
      std::string val = line.substr(colon_pos + 1);

      size_t k_first = key.find_first_not_of(" \r\t");
      if (k_first != std::string::npos) {
        key = key.substr(k_first, key.find_last_not_of(" \r\t") - k_first + 1);
      } else {
        continue;
      }

      size_t v_first = val.find_first_not_of(" \r\t");
      if (v_first != std::string::npos) {
        val = val.substr(v_first, val.find_last_not_of(" \r\t") - v_first + 1);
      } else {
        val = "";
      }

      result[key] = val;
    }
  }
  return result;
}

std::vector<ct::Tag> MarkdownSerializer::ParseTags(
    const std::string& tags_str) {
  std::vector<ct::Tag> res;
  if (tags_str.size() < 2 || tags_str.front() != '[' ||
      tags_str.back() != ']') {
    return res;
  }

  std::string clean = tags_str.substr(1, tags_str.size() - 2);
  if (clean.empty()) {
    return res;
  }

  std::stringstream ss(clean);
  std::string t;
  while (std::getline(ss, t, ',')) {
    size_t first = t.find_first_not_of(" \t\r\n");
    if (first != std::string::npos) {
      size_t last = t.find_last_not_of(" \t\r\n");
      res.emplace_back(t.substr(first, (last - first + 1)));
    }
  }
  return res;
}

}  // namespace infra::serialization
