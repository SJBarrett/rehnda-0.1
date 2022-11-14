//
// Created by sjbar on 16/09/2022.
//

#pragma once


#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda::VkDebugHelpers {

    vkr::DebugUtilsMessengerEXT setupDebugMessenger(vkr::Instance& instance);

    vk::DebugUtilsMessengerCreateInfoEXT build_debug_messenger_create_info();
};
