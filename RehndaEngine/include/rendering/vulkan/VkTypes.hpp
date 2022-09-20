#pragma once

#include <named_type.hpp>
#include <optional>

namespace Rehnda {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsQueueIndex;
        std::optional<uint32_t> presentQueueIndex;

        [[nodiscard]]
        bool requiredFamiliesFound() const {
            return graphicsQueueIndex.has_value() && presentQueueIndex.has_value();
        }
    };
}