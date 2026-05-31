#pragma once

#include <functional>

#include "core/entities/entity.hpp"

namespace app::commands {

using EditAction = std::function<void(core::entities::Entity&)>;

}  // namespace app::commands
