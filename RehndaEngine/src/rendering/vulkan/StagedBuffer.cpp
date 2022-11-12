//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/StagedBuffer.hpp"
#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda {
    StagedBuffer::StagedBuffer(vk::Device &device, vk::PhysicalDevice &physicalDevice, vk::CommandPool &commandPool,
                               vk::Queue &queue, const StagedBufferProps& stagedBufferProps) : dataSize(stagedBufferProps.dataSize), device(device), physicalDevice(physicalDevice) {
        // create the staging buffer which the host needs to be able to see (and coherent ensures the data is the same as what the CPU expects?)
        // and will be transferred from to the gpu later (hence transferSrc)
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        BufferHelper::CreateBufferAndAssignMemoryProps stagingBufferProps{
                .size = dataSize,
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


        // put the data into the staging buffer
        void *mappedMemory;
        assert(device.mapMemory(stagingBufferMemory, 0, stagingBufferProps.size, vk::MemoryMapFlags{}, &mappedMemory) ==
               vk::Result::eSuccess);
        memcpy(mappedMemory, stagedBufferProps.data, (size_t) stagingBufferProps.size);
        device.unmapMemory(stagingBufferMemory);

        // TODO allocating memory for every buffer is not scalable as there is a max mem
        //  allocation count which is relatively low (as low as 4096 on a 1080)
        BufferHelper::CreateBufferAndAssignMemoryProps bufferProps{
                .size = dataSize,
                .bufferUsage = stagedBufferProps.bufferUsageFlags | vk::BufferUsageFlagBits::eTransferDst,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
        };
        BufferHelper::createBuffer(
                device,
                physicalDevice,
                bufferProps,
                buffer,
                bufferMemory
        );

        copyStagedBufferToGpu(stagingBuffer, commandPool, queue);

        device.destroyBuffer(stagingBuffer);
        device.freeMemory(stagingBufferMemory);
    }

    void StagedBuffer::destroy() {
        device.destroyBuffer(buffer);
        device.freeMemory(bufferMemory);
    }


    const vk::Buffer &StagedBuffer::getBuffer() const {
        return buffer;
    }

    void
    StagedBuffer::copyStagedBufferToGpu(vk::Buffer &stagingBuffer, vk::CommandPool &commandPool, vk::Queue &queue) {
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
                        .size = dataSize
                }
        };
        commandBuffer.copyBuffer(stagingBuffer, buffer, bufferCopy);
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