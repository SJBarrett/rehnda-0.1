//
// Created by sjbar on 18/11/2022.
//

#pragma once

#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda {
    class SingleTimeCommand {
    public:
        SingleTimeCommand(vkr::Device &device, vkr::Queue& queue, vkr::CommandPool &commandPool);
        ~SingleTimeCommand();
        vkr::CommandBuffer commandBuffer;
    private:
        vkr::Queue& queue;

        static vkr::CommandBuffer initBuffer(vkr::Device& device, vkr::CommandPool &commandPool);
    };
}
