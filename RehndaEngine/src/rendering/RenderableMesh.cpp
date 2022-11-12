//
// Created by sjbar on 12/11/2022.
//

#include "rendering/RenderableMesh.hpp"

namespace Rehnda {

    RenderableMesh::RenderableMesh(const DeviceContext &deviceContext, const std::vector<Vertex> &vertices,
                                   const std::vector<uint16_t> &indices) :
            vertexBuffer(
                    deviceContext.device, deviceContext.physicalDevice, deviceContext.memoryCommandPool,
                    deviceContext.graphicsQueue, StagedBufferProps{
                            .data = vertices.data(),
                            .dataSize = sizeof(vertices[0]) * vertices.size(),
                            .bufferUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer
                    }),
            indexBuffer(deviceContext.device, deviceContext.physicalDevice, deviceContext.memoryCommandPool,
                        deviceContext.graphicsQueue, StagedBufferProps{
                            .data = indices.data(),
                            .dataSize = sizeof(indices[0]) * indices.size(),
                            .bufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer
                    }),
            indicesCount(indices.size()) {
    }

    void RenderableMesh::draw(vk::CommandBuffer &commandBuffer) const {
        vk::Buffer vertexBuffers[] = {vertexBuffer.getBuffer()};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint16);

        // indices count, instance count
        commandBuffer.drawIndexed(indicesCount, 1, 0, 0, 0);
    }

    void RenderableMesh::destroy() {
        vertexBuffer.destroy();
        indexBuffer.destroy();
    }
}