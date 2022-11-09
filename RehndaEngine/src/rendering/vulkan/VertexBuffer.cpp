//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/VertexBuffer.hpp"

namespace Rehnda {
    VertexBuffer::VertexBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const std::vector<Vertex>& vertices) : device(device), physicalDevice(physicalDevice) {
        vk::BufferCreateInfo bufferCreateInfo {
            .size = sizeof(vertices[0]) * vertices.size(),
            .usage = vk::BufferUsageFlagBits::eVertexBuffer,
            // this buffer is only used by the graphics queue, so can be exclusive
            .sharingMode = vk::SharingMode::eExclusive
        };

        // create the buffer, however the memory hasn't been assigned yet for the buffer
        if (device.createBuffer(&bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create buffer");
        }

        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);

        // we need coherent memory to ensure the mapped memory always matches the contexts of the allocated memory
        // spec guarantees the memory copy will be complete as of the next queueSubmit call
        // alternative is to flush the memory https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkFlushMappedMemoryRanges.html
        vk::MemoryPropertyFlags memoryRequiredProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        vk::MemoryAllocateInfo memoryAllocateInfo {
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryRequiredProperties)
        };

        if (device.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to allocate vertex buffer memory");
        }
        // associate the allocated memory with the previously created buffer
        device.bindBufferMemory(buffer, bufferMemory, 0);

        // put the vertices into the buffer by mapping the memory on the GPU to a memory address we can copy into, and then unmap
        void* data;
        assert(device.mapMemory(bufferMemory, 0, bufferCreateInfo.size, vk::MemoryMapFlags {}, &data) == vk::Result::eSuccess);
        memcpy(data, vertices.data(), (size_t) bufferCreateInfo.size);
        device.unmapMemory(bufferMemory);
    }

    VertexBuffer::~VertexBuffer() {
        device.destroyBuffer(buffer);
        device.freeMemory(bufferMemory);
    }

    uint32_t VertexBuffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
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

    vk::Buffer &VertexBuffer::getBuffer() {
        return buffer;
    }
}