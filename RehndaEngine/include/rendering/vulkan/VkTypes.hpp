#pragma once

#include <named_type.hpp>
#include <optional>

namespace Rehnda {
     using GraphicsQueueIndex = fluent::NamedType<uint32_t , struct GraphicsQueueIndexTag>;
     using PresentQueueIndex = fluent::NamedType<uint32_t , struct PresentQueueIndexTag>;


}