//
// Created by sjbar on 16/09/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include <GLFW/glfw3.h>

#include "VkTypes.hpp"
#include "SwapchainManager.hpp"
#include "GraphicsPipeline.hpp"
#include "TextureImage.hpp"
#include "TextureSampler.hpp"


namespace Rehnda {
    enum class DrawFrameResult {
        SUCCESS,
        SWAPCHAIN_OUT_OF_DATE,
    };

    class FrameCoordinator {
    public:
        FrameCoordinator(GLFWwindow *window, vkr::Device &device, vkr::PhysicalDevice &physicalDevice,
                         vkr::SurfaceKHR &surface,
                         QueueFamilyIndices queueFamilyIndices);

        DrawFrameResult drawFrame();

        void setFramebufferResized();

    private:
        const size_t MAX_FRAMES_IN_FLIGHT = 2;
        size_t currentFrame = 0;
        bool framebufferResized = false;

        vkr::Device &device;
        vkr::PhysicalDevice &physicalDevice;
        QueueFamilyIndices queueFamilyIndices;

        vkr::Queue graphicsQueue;
        vkr::Queue presentQueue;
        vkr::CommandPool graphicsCommandPool;
        vkr::CommandPool memoryCommandPool;
        std::vector<vkr::CommandBuffer> commandBuffers;

        std::vector<vkr::Semaphore> imageAvailableSemaphores;
        std::vector<vkr::Semaphore> renderFinishedSemaphores;
        std::vector<vkr::Fence> inFlightFences;

        // UBOs
        std::vector<WritableDirectBuffer> uboBuffers;
        vkr::DescriptorSetLayout descriptorSetLayout;
        vkr::DescriptorPool descriptorPool;
        vkr::DescriptorSets descriptorSets;

        std::unique_ptr<SwapchainManager> swapchainManager;
        std::unique_ptr<GraphicsPipeline> graphicsPipeline;

        // tempsdf
        std::unique_ptr<RenderableMesh> mesh;
        std::unique_ptr<TextureImage> textureImage;
        std::unique_ptr<TextureSampler> textureSampler;
    private:
        vkr::CommandPool createCommandPool(vk::CommandPoolCreateFlags commandPoolCreateFlags);

        std::vector<vkr::Semaphore> createSemaphores(size_t numToCreate);
        std::vector<vkr::Fence> createFences(size_t numToCreate);

        vkr::CommandBuffers createCommandBuffers();

        vkr::DescriptorSetLayout createDescriptorSetLayout();

        std::vector<WritableDirectBuffer> createUbos();

        vkr::DescriptorPool createDescriptorPool();

        vkr::DescriptorSets createDescriptorSets();

        void updateUniformBuffer(uint32_t currentImage);
    };
}
