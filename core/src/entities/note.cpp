#include "core/entities/note.hpp"

namespace core::entities {

std::unique_ptr<Entity> Note::Clone() const {
  auto clone = std::make_unique<Note>(meta_, content_);

  for (const auto& child : children_) {
    clone->AddChild(child->Clone());
  }

  return clone;
}

}  // namespace core::entities
