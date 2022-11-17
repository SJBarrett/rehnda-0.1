//
// Created by sjbar on 10/11/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include "rendering/Vertex.hpp"
#include "core/CoreTypes.hpp"

namespace Rehnda {
    struct StagedBufferProps {
        const void *data;
        vk::DeviceSize dataSize;
        vk::BufferUsageFlags bufferUsageFlags;
    };

    class StagedBuffer {
    public:
        StagedBuffer(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vkr::CommandPool &commandPool, vkr::Queue &queue,
                     const StagedBufferProps &stagedBufferProps);

        StagedBuffer(const StagedBuffer &) = delete;

        [[nodiscard]]
        const vkr::Buffer& getBuffer() const;

    private:
        vk::DeviceSize dataSize;
        vkr::Device &device;
        vkr::PhysicalDevice &physicalDevice;

        vkr::Buffer buffer;
        vkr::DeviceMemory bufferMemory;

    private:
        vkr::Buffer initBuffer( vk::BufferUsageFlags bufferUsageFlags);
        vkr::DeviceMemory initBufferMemory();
    };
}
