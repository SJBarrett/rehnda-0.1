//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/WritableDirectBuffer.hpp"
#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda {
    WritableDirectBuffer::WritableDirectBuffer(vkr::Device &device, vkr::PhysicalDevice &physicalDevice,
                                               const WritableDirectBufferProps &directBufferProps) : dataSize(
            directBufferProps.dataSize),
                                                                                                     device(device),
                                                                                                     physicalDevice(physicalDevice),
                                                                                                     buffer(initBuffer(directBufferProps.bufferUsageFlags)),
                                                                                                     bufferMemory(initDeviceMemory()) {
        // create the staging buffer which the host needs to be able to see (and coherent ensures the data is the same as what the CPU expects?)
        // and will be transferred from to the gpu later (hence transferSrc)
        buffer.bindMemory(*bufferMemory, 0);
        mappedMemory = bufferMemory.mapMemory(0, dataSize, vk::MemoryMapFlags{});
        if (directBufferProps.data != nullptr) {
            writeData(directBufferProps.data);
        }
        // don't unmap the memory so we can write to it whenever we want
    }

    vkr::DeviceMemory WritableDirectBuffer::initDeviceMemory() {
        vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

        vk::MemoryAllocateInfo memoryAllocateInfo{
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex = BufferHelper::findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits,
                                                                vk::MemoryPropertyFlagBits::eHostVisible |
                                                                vk::MemoryPropertyFlagBits::eHostCoherent)
        };
        return {device, memoryAllocateInfo};
    }

    const vkr::Buffer &WritableDirectBuffer::getBuffer() const {
        return buffer;
    }

    void WritableDirectBuffer::writeData(const void *data) {
        memcpy(mappedMemory, data, (size_t) dataSize);
    }

    vkr::Buffer WritableDirectBuffer::initBuffer(vk::BufferUsageFlags bufferUsageFlags) {
        vk::BufferCreateInfo bufferCreateInfo {
                .size=dataSize,
                .usage = bufferUsageFlags,
                .sharingMode = vk::SharingMode::eExclusive
        };
        return {device, bufferCreateInfo};
    }
}