//
// Created by sjbar on 10/11/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include "rendering/Vertex.hpp"

namespace Rehnda {
    struct WritableDirectBufferProps {
        vk::DeviceSize dataSize;
        vk::BufferUsageFlags bufferUsageFlags;
        const void* data = nullptr;
    };
    class WritableDirectBuffer {
    public:
        WritableDirectBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const WritableDirectBufferProps& stagedBufferProps);

        // TODO Make RAII work so we don't have to manually call destroy methods
        void destroy();

        void writeData(const void* data);

        [[nodiscard]]
        const vk::Buffer& getBuffer() const;

    private:
        vk::DeviceSize dataSize;
        vk::Device& device;
        vk::PhysicalDevice& physicalDevice;

        void* mappedMemory;
        vk::Buffer buffer;
        vk::DeviceMemory bufferMemory;
    };
}
