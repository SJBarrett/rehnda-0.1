//
// Created by sjbar on 11/11/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>

namespace Rehnda::BufferHelper {

    struct CreateBufferAndAssignMemoryProps {
        vk::DeviceSize size;
        vk::BufferUsageFlags bufferUsage;
        vk::MemoryPropertyFlags requiredMemoryProperties;
    };

    void createBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const CreateBufferAndAssignMemoryProps& props, vk::Buffer& outBuffer, vk::DeviceMemory& outBufferMemory);

    uint32_t findMemoryType(vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
} // Rehnda