#pragma once

#include <named_type.hpp>
#include <optional>

namespace Rehnda {
     using GraphicsQueueIndex = fluent::NamedType<uint32_t , struct GraphicsQueueIndexTag>;

     struct QueueFamilyIndices {
         std::optional<GraphicsQueueIndex> graphicsQueueIndex;

         bool requiredFamiliesFound() {
             return graphicsQueueIndex.has_value();
         }
     };
}