//
// Created by sjbar on 10/11/2022.
//

#include "rendering/vulkan/WritableDirectBuffer.hpp"
#include "rendering/vulkan/BufferHelper.hpp"

namespace Rehnda {
    WritableDirectBuffer::WritableDirectBuffer(vk::Device &device, vk::PhysicalDevice &physicalDevice,
                                                   const WritableDirectBufferProps &directBufferProps) : dataSize(directBufferProps.dataSize),
                                                                                       device(device),
                                                                                       physicalDevice(physicalDevice) {
        // create the staging buffer which the host needs to be able to see (and coherent ensures the data is the same as what the CPU expects?)
        // and will be transferred from to the gpu later (hence transferSrc)
        BufferHelper::CreateBufferAndAssignMemoryProps bufferProps{
                .size = dataSize,
                .bufferUsage = directBufferProps.bufferUsageFlags,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent
        };
        BufferHelper::createBuffer(
                device,
                physicalDevice,
                bufferProps,
                buffer,
                bufferMemory
        );


        // put the data into the buffer
        assert(device.mapMemory(bufferMemory, 0, dataSize, vk::MemoryMapFlags{}, &mappedMemory) ==
               vk::Result::eSuccess);
        if (directBufferProps.data != nullptr) {
            writeData(directBufferProps.data);
        }
        // don't unmap the memory so we can write to it whenever we want
    }

    void WritableDirectBuffer::destroy() {
        device.destroyBuffer(buffer);
        device.freeMemory(bufferMemory);
    }


    const vk::Buffer &WritableDirectBuffer::getBuffer() const {
        return buffer;
    }

    void WritableDirectBuffer::writeData(const void *data) {
        memcpy(mappedMemory, data, (size_t) dataSize);
    }
}