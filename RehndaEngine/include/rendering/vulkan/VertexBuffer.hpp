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
        VertexBuffer(vk::Device& device, vk::PhysicalDevice& physicalDevice, const std::vector<Vertex>& vertices);
        VertexBuffer(const VertexBuffer&) = delete;
        ~VertexBuffer();

        vk::Buffer& getBuffer();

    private:
        vk::Device& device;
        vk::PhysicalDevice& physicalDevice;

        vk::Buffer buffer;
        vk::DeviceMemory bufferMemory;

        [[nodiscard]]
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
    };
}
