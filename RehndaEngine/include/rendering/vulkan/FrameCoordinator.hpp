//
// Created by sjbar on 16/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VkTypes.hpp"
#include "SwapchainManager.hpp"
#include "GraphicsPipeline.hpp"


namespace Rehnda {
    class FrameCoordinator {
    public:
        FrameCoordinator(NonOwner<vk::Device*> device, NonOwner<SwapchainManager*> swapchainManager, QueueFamilyIndices queueFamilyIndices);
        ~FrameCoordinator();
        void destroy();

        void drawFrame();

    private:
        NonOwner<vk::Device*> device;
        NonOwner<SwapchainManager*> swapchainManager;
        QueueFamilyIndices queueFamilyIndices;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::CommandPool graphicsCommandPool;
        vk::CommandBuffer commandBuffer;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        vk::Fence inFlightFence;

        std::unique_ptr<GraphicsPipeline> graphicsPipeline;

        bool destroyed = false;
    private:
        void initCommandPool();

        void createSyncObjects();

        vk::CommandBuffer createCommandBuffer();
    };
}
