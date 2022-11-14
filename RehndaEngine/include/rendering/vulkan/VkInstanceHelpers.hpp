//
// Created by sjbar on 3/09/2022.
//

#pragma once

#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda::VkInstanceHelpers {
    vkr::Instance buildVulkanInstance(vkr::Context& context, std::vector<const char *> validationLayers);

    bool are_validation_layers_supported(vkr::Context &context, const std::vector<const char *> &validationLayers);

    std::vector<const char *> get_required_extensions(std::vector<const char *> vector);

}
