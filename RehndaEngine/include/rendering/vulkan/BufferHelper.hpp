//
// Created by sjbar on 11/11/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda::BufferHelper {
    struct CreateBufferAndAssignMemoryProps {
        vk::DeviceSize size;
        vk::BufferUsageFlags bufferUsage;
        vk::MemoryPropertyFlags requiredMemoryProperties;
    };

    std::tuple<vkr::Buffer, vkr::DeviceMemory> createBuffer(vkr::Device& device, vkr::PhysicalDevice& physicalDevice, const CreateBufferAndAssignMemoryProps& props);

    uint32_t findMemoryType(vkr::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
} // Rehnda