//
// Created by sjbar on 10/11/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include "rendering/Vertex.hpp"

namespace Rehnda {
    struct WritableDirectBufferProps {
        vk::DeviceSize dataSize;
        vk::BufferUsageFlags bufferUsageFlags;
        const void *data = nullptr;
    };

    class WritableDirectBuffer {
    public:
        WritableDirectBuffer(vkr::Device &device, vkr::PhysicalDevice &physicalDevice,
                             const WritableDirectBufferProps &stagedBufferProps);

        void writeData(const void *data);

        [[nodiscard]]
        const vkr::Buffer &getBuffer() const;

    private:
        vk::DeviceSize dataSize;
        vkr::Device &device;
        vkr::PhysicalDevice &physicalDevice;

        void *mappedMemory;
        vkr::Buffer buffer;
        vkr::DeviceMemory bufferMemory;

        vkr::Buffer initBuffer(vk::BufferUsageFlags bufferUsageFlags);
        vkr::DeviceMemory initDeviceMemory();
    };
}
