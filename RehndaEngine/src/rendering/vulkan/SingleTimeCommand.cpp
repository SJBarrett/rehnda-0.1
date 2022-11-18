//
// Created by sjbar on 18/11/2022.
//

#include "rendering/vulkan/SingleTimeCommand.hpp"

namespace Rehnda {

    SingleTimeCommand::SingleTimeCommand(vkr::Device &device, vkr::Queue& queue, vkr::CommandPool &commandPool) : commandBuffer(initBuffer(device, commandPool)), queue(queue) {
        commandBuffer.begin(vk::CommandBufferBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        });
    }

    SingleTimeCommand::~SingleTimeCommand() {
        commandBuffer.end();
        queue.submit(vk::SubmitInfo{
            .commandBufferCount = 1,
            .pCommandBuffers = &*commandBuffer
        });
        queue.waitIdle();
    }

    vkr::CommandBuffer SingleTimeCommand::initBuffer(vkr::Device& device, vkr::CommandPool &commandPool) {
        vk::CommandBufferAllocateInfo allocateInfo{
                .commandPool = *commandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
        };
        vkr::CommandBuffers commandBuffers{device, allocateInfo};
        return std::move(commandBuffers[0]);
    }
}