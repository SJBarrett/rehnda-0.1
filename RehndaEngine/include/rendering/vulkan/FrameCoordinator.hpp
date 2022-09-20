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
        const size_t MAX_FRAMES_IN_FLIGHT = 2;
        size_t currentFrame = 0;

        NonOwner<vk::Device*> device;
        NonOwner<SwapchainManager*> swapchainManager;
        QueueFamilyIndices queueFamilyIndices;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::CommandPool graphicsCommandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;

        std::unique_ptr<GraphicsPipeline> graphicsPipeline;

        bool destroyed = false;
    private:
        void initCommandPool();

        void createSyncObjects();

        void createCommandBuffers();
    };
}
