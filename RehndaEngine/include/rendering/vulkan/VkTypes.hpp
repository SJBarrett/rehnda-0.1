#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_CPP_VERSION 20
#include <utility>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

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

    namespace vkr = vk::raii;
}