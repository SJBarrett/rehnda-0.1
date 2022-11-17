//
// Created by sjbar on 11/11/2022.
//

#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda::BufferHelper {
    std::tuple<vkr::Buffer, vkr::DeviceMemory>
    createBuffer(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, const CreateBufferAndAssignMemoryProps &props) {
        vk::BufferCreateInfo bufferCreateInfo{
                .size = props.size,
                .usage = props.bufferUsage,
                // this buffer is only used by the graphics queue, so can be exclusive
                .sharingMode = vk::SharingMode::eExclusive
        };
        vkr::Buffer outBuffer{device, bufferCreateInfo};


        vk::MemoryRequirements memoryRequirements = outBuffer.getMemoryRequirements();

        vk::MemoryAllocateInfo memoryAllocateInfo{
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, props.requiredMemoryProperties)
        };
        vkr::DeviceMemory bufferMemory{device, memoryAllocateInfo};
        // associate the allocated memory with the previously created buffer
        outBuffer.bindMemory(*bufferMemory, 0);

        return {std::move(outBuffer), std::move(bufferMemory)};
    }

    uint32_t findMemoryType(vkr::PhysicalDevice &physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            bool isCorrectType = typeFilter & (1 << i);
            bool hasSuitableProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
            if (isCorrectType && hasSuitableProperties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory for a vertex buffer");
    }
} // Rehnda