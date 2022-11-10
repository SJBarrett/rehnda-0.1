//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/VertexBuffer.hpp"
#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda {
    VertexBuffer::VertexBuffer(vk::Device &device, vk::PhysicalDevice &physicalDevice, vk::CommandPool &commandPool,
                               vk::Queue &queue,
                               const std::vector<Vertex> &vertices) : verticesSize(
            sizeof(vertices[0]) * vertices.size()), device(device), physicalDevice(physicalDevice) {
        // create the staging vertexBuffer which the host needs to be able to see (and coherent ensures the data is the same as what the CPU expects?)
        // and will be transferred from to the gpu later (hence transferSrc)
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        BufferHelper::CreateBufferAndAssignMemoryProps stagingBufferProps{
                .size = verticesSize,
                .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent
        };
        BufferHelper::createBuffer(
                device,
                physicalDevice,
                stagingBufferProps,
                stagingBuffer,
                stagingBufferMemory
        );


        // put the vertices into the staging vertexBuffer
        void *data;
        assert(device.mapMemory(stagingBufferMemory, 0, stagingBufferProps.size, vk::MemoryMapFlags{}, &data) ==
               vk::Result::eSuccess);
        memcpy(data, vertices.data(), (size_t) stagingBufferProps.size);
        device.unmapMemory(stagingBufferMemory);

        // TODO allocating memory for every vertex buffer is not scalable as there is a max mem
        //  allocation count which is relatively low (as low as 4096 on a 1080)
        BufferHelper::CreateBufferAndAssignMemoryProps vertexBufferProps{
                .size = sizeof(vertices[0]) * vertices.size(),
                .bufferUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
        };
        BufferHelper::createBuffer(
                device,
                physicalDevice,
                vertexBufferProps,
                vertexBuffer,
                vertexBufferMemory
        );

        copyStagedBufferToGpu(stagingBuffer, commandPool, queue);

        device.destroyBuffer(stagingBuffer);
        device.freeMemory(stagingBufferMemory);
    }

    VertexBuffer::~VertexBuffer() {
        device.destroyBuffer(vertexBuffer);
        device.freeMemory(vertexBufferMemory);
    }


    vk::Buffer &VertexBuffer::getBuffer() {
        return vertexBuffer;
    }

    void
    VertexBuffer::copyStagedBufferToGpu(vk::Buffer &stagingBuffer, vk::CommandPool &commandPool, vk::Queue &queue) {
        vk::CommandBufferAllocateInfo allocateInfo{
                .commandPool = commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
        };

        vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];

        vk::CommandBufferBeginInfo beginInfo{
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        commandBuffer.begin(beginInfo);

        std::array<vk::BufferCopy, 1> bufferCopy{
                vk::BufferCopy{
                        .srcOffset = 0,
                        .dstOffset = 0,
                        .size = verticesSize
                }
        };
        commandBuffer.copyBuffer(stagingBuffer, vertexBuffer, bufferCopy);
        commandBuffer.end();

        queue.submit({vk::SubmitInfo{
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        }});
        // Instead of waiting on the queue to confirm the memory has been copied can use fences to queue multiple
        // transfers at once and then wait for all of them
        queue.waitIdle();
        device.freeCommandBuffers(commandPool, {commandBuffer});
    }
}