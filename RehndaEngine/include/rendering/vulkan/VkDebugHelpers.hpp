//
// Created by sjbar on 16/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace Rehnda::VkDebugHelpers {

    void setupDebugMessenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT* debugMessenger, const vk::AllocationCallbacks* allocator = nullptr);
    void destroy_debug_messenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT debugMessenger, const vk::AllocationCallbacks* allocator = nullptr);

    vk::DebugUtilsMessengerCreateInfoEXT build_debug_messenger_create_info();
};
