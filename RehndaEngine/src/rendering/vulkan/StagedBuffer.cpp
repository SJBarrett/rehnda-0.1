//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/StagedBuffer.hpp"
#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda {
    StagedBuffer::StagedBuffer(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vkr::CommandPool &commandPool,
                               vkr::Queue &queue, const StagedBufferProps &stagedBufferProps) :
            dataSize(stagedBufferProps.dataSize),
            device(device),
            physicalDevice(physicalDevice),
            buffer(initBuffer(stagedBufferProps.bufferUsageFlags)),
            bufferMemory(initBufferMemory()) {
        buffer.bindMemory(*bufferMemory, 0);
        // create the staging buffer which the host needs to be able to see (and coherent ensures the data is the same as what the CPU expects?)
        // and will be transferred from to the gpu later (hence transferSrc)
        BufferHelper::CreateBufferAndAssignMemoryProps stagingBufferProps{
                .size = dataSize,
                .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent
        };
        auto [stagingBuffer, stagingBufferMemory] = BufferHelper::createBuffer(
                device,
                physicalDevice,
                stagingBufferProps
        );


        // put the data into the staging buffer
        void *mappedMemory = stagingBufferMemory.mapMemory(0, stagedBufferProps.dataSize, vk::MemoryMapFlags{});
        memcpy(mappedMemory, stagedBufferProps.data, (size_t) stagingBufferProps.size);
        stagingBufferMemory.unmapMemory();

        // TODO#2 allocating memory for every buffer is not scalable as there is a max mem
        //  allocation count which is relatively low (as low as 4096 on a 1080)
        vk::CommandBufferAllocateInfo allocateInfo{
                .commandPool = *commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
        };

        vkr::CommandBuffers commandBuffers{device, allocateInfo};
        vkr::CommandBuffer &commandBuffer = commandBuffers.front();

        vk::CommandBufferBeginInfo beginInfo{
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };
        commandBuffer.begin(beginInfo);

        vk::BufferCopy bufferCopy{
                .srcOffset = 0,
                .dstOffset = 0,
                .size = dataSize
        };
        commandBuffer.copyBuffer(*stagingBuffer, *buffer, bufferCopy);
        commandBuffer.end();

        queue.submit({vk::SubmitInfo{
                .commandBufferCount = 1,
                .pCommandBuffers = &*commandBuffer
        }});
        // Instead of waiting on the queue to confirm the memory has been copied can use fences to queue multiple
        // transfers at once and then wait for all of them
        queue.waitIdle();
    }

    const vkr::Buffer &StagedBuffer::getBuffer() const {
        return buffer;
    }

    vkr::Buffer StagedBuffer::initBuffer(vk::BufferUsageFlags bufferUsageFlags) {
        vk::BufferCreateInfo bufferCreateInfo{
                .size = dataSize,
                .usage = bufferUsageFlags | vk::BufferUsageFlagBits::eTransferDst,
                // this buffer is only used by the graphics queue, so can be exclusive
                .sharingMode = vk::SharingMode::eExclusive
        };
        return {device, bufferCreateInfo};
    }

    vkr::DeviceMemory StagedBuffer::initBufferMemory() {
        vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{
                .allocationSize = dataSize,
                .memoryTypeIndex = BufferHelper::findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits,
                                                                vk::MemoryPropertyFlagBits::eDeviceLocal)
        };
        return {device, memoryAllocateInfo};
    }
}