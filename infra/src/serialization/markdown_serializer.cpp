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

  std::string extra_fields;
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
  const std::string sep = "---";
  size_t first_sep = raw_content.find(sep);
  size_t second_sep = raw_content.find(sep, first_sep + sep.length());

  if (first_sep == std::string::npos || second_sep == std::string::npos) {
    throw ct::ValidationError("Markdown",
                              "Invalid file format: YAML header missing");
  }

  std::string yaml = raw_content.substr(
      first_sep + sep.length(), second_sep - (first_sep + sep.length()));

  size_t end_of_header_line = raw_content.find('\n', second_sep + sep.length());
  std::string content = "";
  if (end_of_header_line != std::string::npos) {
    content = raw_content.substr(end_of_header_line + 1);
  }

  ct::ID id(ExtractYamlValue(yaml, "id"));
  std::string type = ExtractYamlValue(yaml, "type");
  ct::Title title(ExtractYamlValue(yaml, "title"));
  auto tags = ParseTags(ExtractYamlValue(yaml, "tags"));
  auto created =
      ct::Timestamp::FromIsoString(ExtractYamlValue(yaml, "created_at"));
  auto updated =
      ct::Timestamp::FromIsoString(ExtractYamlValue(yaml, "updated_at"));

  std::string p_raw = ExtractYamlValue(yaml, "parent");
  std::optional<ct::ID> parent_id;
  if (!p_raw.empty() && p_raw != "null") {
    parent_id = ct::ID(p_raw);
  }

  ce::Metadata meta(id, title, tags);
  meta.created_at = created;
  meta.updated_at = updated;

  std::unique_ptr<ce::Entity> entity;
  if (type == "task") {
    std::string s_str = ExtractYamlValue(yaml, "status");
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

std::string MarkdownSerializer::ExtractYamlValue(const std::string& yaml,
                                                 const std::string& key) {
  std::string full_key = key + ":";
  size_t pos = yaml.find(full_key);

  while (pos != std::string::npos) {
    if (pos == 0 || yaml[pos - 1] == '\n') {
      size_t val_start = pos + full_key.length();
      size_t val_end = yaml.find('\n', val_start);

      std::string val;
      if (val_end == std::string::npos) {
        val = yaml.substr(val_start);
      } else {
        val = yaml.substr(val_start, val_end - val_start);
      }

      size_t first = val.find_first_not_of(" ");
      if (first == std::string::npos) {
        return "";
      }
      size_t last = val.find_last_not_of(" \r");
      return val.substr(first, (last - first + 1));
    }
    pos = yaml.find(full_key, pos + 1);
  }
  return "";
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
    size_t first = t.find_first_not_of(" ");
    if (first != std::string::npos) {
      size_t last = t.find_last_not_of(" ");
      res.emplace_back(t.substr(first, (last - first + 1)));
    }
  }
  return res;
}

}  // namespace infra::serialization