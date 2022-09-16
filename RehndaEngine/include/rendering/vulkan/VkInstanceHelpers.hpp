//
// Created by sjbar on 3/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace Rehnda::VkInstanceHelpers {
    vk::Instance buildVulkanInstance(std::vector<const char *> validationLayers);

    bool are_validation_layers_supported(const std::vector<const char *> &validationLayers);

    std::vector<const char *> get_required_extensions(std::vector<const char *> vector);

}
