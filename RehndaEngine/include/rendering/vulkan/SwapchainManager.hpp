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

    enum class PresentResult {
        SUCCESS,
        SWAPCHAIN_OUT_OF_DATE,
    };

    class SwapchainManager {
    public:
        SwapchainManager(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice, vk::Device *device,
                         const vk::SurfaceKHR &surface, QueueFamilyIndices queueFamilyIndices);

        ~SwapchainManager();

        void resize(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface, QueueFamilyIndices indices, const vk::RenderPass &renderPass);

        void initSwapchainBuffers(const vk::RenderPass &renderPass);

        void destroy();

        vk::ResultValue<uint32_t> acquireNextImageIndex(vk::Semaphore &imageAvailableSemaphore);

        PresentResult present(const std::vector<vk::Semaphore> &waitSemaphores, vk::Queue &presentQueue, uint32_t imageIndex);

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

        NonOwner<vk::Device *> device;

    private:
        void cleanupResources();

        void createImageViews();

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

        vk::Extent2D chooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities);

        void createSwapchain(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice, const vk::Device *device,
                             const vk::SurfaceKHR &surface, QueueFamilyIndices &indices);
    };
}