//
// Created by sjbar on 10/11/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include "rendering/Vertex.hpp"

namespace Rehnda {
    class VertexBuffer {
    public:
        VertexBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, vk::CommandPool& commandPool, vk::Queue& queue, const std::vector<Vertex>& vertices);
        VertexBuffer(const VertexBuffer&) = delete;

        // TODO Make RAII work so we don't have to manually call destroy methods
        void destroy();

        vk::Buffer& getBuffer();

    private:
        vk::DeviceSize verticesSize;
        vk::Device& device;
        vk::PhysicalDevice& physicalDevice;

        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;

    private:
        void copyStagedBufferToGpu(vk::Buffer& stagingBuffer, vk::CommandPool& commandPool, vk::Queue& queue);
    };
}
