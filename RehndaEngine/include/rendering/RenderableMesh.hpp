//
// Created by sjbar on 12/11/2022.
//

#pragma once

#include <vector>
#include <cstdint>
#include "Vertex.hpp"
#include "rendering/vulkan/StagedBuffer.hpp"

namespace Rehnda {
    struct DeviceContext {
        vk::Device& device;
        vk::PhysicalDevice& physicalDevice;
        vk::CommandPool& memoryCommandPool;
        vk::Queue& graphicsQueue;
    };
    class RenderableMesh {
    public:
        RenderableMesh(const DeviceContext& deviceContext, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
        RenderableMesh(const RenderableMesh&) =delete;
        RenderableMesh& operator=(const RenderableMesh&) =delete;

        void draw(vk::CommandBuffer& commandBuffer) const;

        void destroy();

    private:
        StagedBuffer vertexBuffer;
        StagedBuffer indexBuffer;
        uint32_t indicesCount;
    };
}