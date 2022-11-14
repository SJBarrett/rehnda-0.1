//
// Created by sjbar on 16/09/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include <GLFW/glfw3.h>

#include "VkTypes.hpp"
#include "SwapchainManager.hpp"
#include "GraphicsPipeline.hpp"


namespace Rehnda {
    enum class DrawFrameResult {
        SUCCESS,
        SWAPCHAIN_OUT_OF_DATE,
    };

    class FrameCoordinator {
    public:
        FrameCoordinator(GLFWwindow *window, vk::Device &device, vk::PhysicalDevice &physicalDevice,
                         vk::SurfaceKHR &surface,
                         QueueFamilyIndices queueFamilyIndices);

        ~FrameCoordinator();

//        void destroy();

        DrawFrameResult drawFrame();

        void setFramebufferResized();

    private:
        const size_t MAX_FRAMES_IN_FLIGHT = 2;
        size_t currentFrame = 0;
        bool framebufferResized = false;

        vk::Device &device;
        vk::PhysicalDevice &physicalDevice;
        std::unique_ptr<SwapchainManager> swapchainManager;
        QueueFamilyIndices queueFamilyIndices;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::CommandPool graphicsCommandPool;
        vk::CommandPool memoryCommandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;

        // UBOs
        std::vector<WritableDirectBuffer> uboBuffers;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;

        std::unique_ptr<GraphicsPipeline> graphicsPipeline;

        // temp
        std::unique_ptr<RenderableMesh> mesh;
    private:
        void initCommandPool();

        void createSyncObjects();

        void createCommandBuffers();

        void createDescriptorSetLayout();

        void createUbos();

        void createDescriptorPool();

        void createDescriptorSets();

        void updateUniformBuffer(uint32_t currentImage);
    };
}
