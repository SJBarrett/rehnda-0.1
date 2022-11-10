//
// Created by sjbar on 10/11/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include "rendering/Vertex.hpp"

namespace Rehnda {
    struct StagedBufferProps {
        const void* data;
        vk::DeviceSize dataSize;
        vk::BufferUsageFlags bufferUsageFlags;
    };
    class StagedBuffer {
    public:
        StagedBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, vk::CommandPool& commandPool, vk::Queue& queue, const StagedBufferProps& stagedBufferProps);
        StagedBuffer(const StagedBuffer&) = delete;

        // TODO Make RAII work so we don't have to manually call destroy methods
        void destroy();

        vk::Buffer& getBuffer();

    private:
        vk::DeviceSize dataSize;
        vk::Device& device;
        vk::PhysicalDevice& physicalDevice;

        vk::Buffer buffer;
        vk::DeviceMemory bufferMemory;

    private:
        void copyStagedBufferToGpu(vk::Buffer& stagingBuffer, vk::CommandPool& commandPool, vk::Queue& queue);
    };
}
