//
// Created by sjbar on 18/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "core/CoreTypes.hpp"
#include "VkTypes.hpp"


namespace Rehnda {
    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class SwapchainManager {
    public:
        SwapchainManager(GLFWwindow *window, const vk::PhysicalDevice& physicalDevice, vk::Device* device, const vk::SurfaceKHR& surface, QueueFamilyIndices queueFamilyIndices);
        ~SwapchainManager();
        void initSwapchainBuffers(const vk::RenderPass& renderPass);
        void destroy();
        uint32_t acquireNextImageIndex(vk::Semaphore& imageAvailableSemaphore);
        void present(const std::vector<vk::Semaphore>& waitSemaphores, vk::Queue& presentQueue, uint32_t imageIndex);

        vk::Extent2D getExtent() const;
        vk::Format getImageFormat() const;
        [[nodiscard]]
        vk::Framebuffer getSwapchainFramebuffer(size_t bufferIndex) const;

        static SwapChainSupportDetails
        querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface);


    private:
        bool destroyed = false;

        vk::SwapchainKHR swapchain;
        vk::Format swapchainImageFormat;
        vk::Extent2D swapchainExtent;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;
        std::vector<vk::Framebuffer> swapchainFramebuffers;

        NonOwner<vk::Device*> device;

    private:
        void createImageViews();

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

        vk::Extent2D chooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities);
    };
}