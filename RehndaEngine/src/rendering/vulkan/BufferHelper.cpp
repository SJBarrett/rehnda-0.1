//
// Created by sjbar on 11/11/2022.
//

#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda::BufferHelper {
    void createBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const CreateBufferAndAssignMemoryProps& props, vk::Buffer& outBuffer, vk::DeviceMemory& outBufferMemory) {
        vk::BufferCreateInfo bufferCreateInfo {
                .size = props.size,
                .usage = props.bufferUsage,
                // this vertexBuffer is only used by the graphics queue, so can be exclusive
                .sharingMode = vk::SharingMode::eExclusive
        };

        // create the vertexBuffer, however the memory hasn't been assigned yet for the vertexBuffer
        if (device.createBuffer(&bufferCreateInfo, nullptr, &outBuffer) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create vertexBuffer");
        }

        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(outBuffer);

        vk::MemoryAllocateInfo memoryAllocateInfo {
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, props.requiredMemoryProperties)
        };

        if (device.allocateMemory(&memoryAllocateInfo, nullptr, &outBufferMemory) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to allocate vertex vertexBuffer memory");
        }
        // associate the allocated memory with the previously created vertexBuffer
        device.bindBufferMemory(outBuffer, outBufferMemory, 0);
    }

    uint32_t findMemoryType(vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            bool isCorrectType = typeFilter & (1 << i);
            bool hasSuitableProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
            if (isCorrectType && hasSuitableProperties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory for a vertex vertexBuffer");
    }
} // Rehnda