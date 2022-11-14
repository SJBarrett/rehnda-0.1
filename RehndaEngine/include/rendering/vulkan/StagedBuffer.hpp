//
// Created by sjbar on 10/11/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include "rendering/Vertex.hpp"

namespace Rehnda {
    struct StagedBufferProps {
        const void *data;
        vk::DeviceSize dataSize;
        vk::BufferUsageFlags bufferUsageFlags;
    };

    class StagedBuffer {
    public:
        StagedBuffer(vk::Device &device, vk::PhysicalDevice &physicalDevice, vk::CommandPool &commandPool, vk::Queue &queue,
                     const StagedBufferProps &stagedBufferProps);

        StagedBuffer(const StagedBuffer &) = delete;
        ~StagedBuffer();

        [[nodiscard]]
        const vk::Buffer &getBuffer() const;

    private:
        vk::DeviceSize dataSize;
        vk::Device &device;
        vk::PhysicalDevice &physicalDevice;

        vk::Buffer buffer;
        vk::DeviceMemory bufferMemory;

    private:
        void copyStagedBufferToGpu(vk::Buffer &stagingBuffer, vk::CommandPool &commandPool, vk::Queue &queue);

        void destroy();
    };
}
