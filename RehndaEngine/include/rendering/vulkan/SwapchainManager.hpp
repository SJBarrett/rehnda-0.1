//
// Created by sjbar on 18/09/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include <GLFW/glfw3.h>
#include "core/CoreTypes.hpp"
#include "VkTypes.hpp"


namespace Rehnda {
    class SwapChainSupportDetails {
    public:
        SwapChainSupportDetails(GLFWwindow *window, const vkr::PhysicalDevice &physicalDevice,
                                const vkr::SurfaceKHR &surface);

        [[nodiscard]]
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat() const;

        [[nodiscard]]
        vk::PresentModeKHR chooseSwapPresentMode() const;

        [[nodiscard]]
        vk::Extent2D chooseSwapExtent() const;

        const vk::SurfaceCapabilitiesKHR capabilities;
        const std::vector<vk::SurfaceFormatKHR> formats;
        const std::vector<vk::PresentModeKHR> presentModes;
    private:
        NonOwner<GLFWwindow *> window;
    };

    enum class PresentResult {
        SUCCESS,
        SWAPCHAIN_OUT_OF_DATE,
    };

    class SwapchainManager {
    public:
        SwapchainManager(vkr::Device &device,
                         const vkr::SurfaceKHR &surface, QueueFamilyIndices, const vkr::RenderPass &renderPass, const vkr::ImageView& depthImageView,
                         const SwapChainSupportDetails &swapChainSupportDetails);

        void resize(const vkr::RenderPass &renderPass, const vkr::ImageView& depthImageView);

        std::pair<vk::Result, uint32_t> acquireNextImageIndex(vkr::Semaphore &imageAvailableSemaphore);

        PresentResult
        present(const std::vector<vk::Semaphore> &waitSemaphores, vkr::Queue &presentQueue, uint32_t imageIndex);

        [[nodiscard]]
        vk::Extent2D getExtent() const;

        [[nodiscard]]
        vkr::Framebuffer &getSwapchainFramebuffer(size_t bufferIndex);


    private:
        vkr::Device &device;
        const vkr::SurfaceKHR &surface;
        const QueueFamilyIndices queueFamilyIndices;
        const SwapChainSupportDetails &swapChainSupportDetails;

        vk::SurfaceFormatKHR swapchainSurfaceFormat;
        vk::Extent2D swapchainExtent;
        std::unique_ptr<vkr::SwapchainKHR> swapchain;
        std::vector<vkr::ImageView> swapchainImageViews;
        std::vector<vkr::Framebuffer> swapchainFramebuffers;


    private:
        std::vector<vkr::ImageView> createImageViews();

        std::unique_ptr<vkr::SwapchainKHR> createSwapchain();

        std::vector<vkr::Framebuffer> createFrameBuffers(const vkr::RenderPass &renderPass, const vkr::ImageView& depthImageView);
    };
}