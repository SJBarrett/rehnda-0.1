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
        SwapChainSupportDetails(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice,
                                const vk::SurfaceKHR &surface);

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
        SwapchainManager(vk::Device &device,
                         const vk::SurfaceKHR &surface, QueueFamilyIndices, const vk::RenderPass &renderPass,
                         const SwapChainSupportDetails &swapChainSupportDetails);

        ~SwapchainManager();

        void resize(const vk::RenderPass &renderPass);

        vk::ResultValue<uint32_t> acquireNextImageIndex(vk::Semaphore &imageAvailableSemaphore);

        PresentResult
        present(const std::vector<vk::Semaphore> &waitSemaphores, vk::Queue &presentQueue, uint32_t imageIndex);

        [[nodiscard]]
        vk::Extent2D getExtent() const;

        [[nodiscard]]
        vk::Framebuffer &getSwapchainFramebuffer(size_t bufferIndex);


    private:
        vk::Device &device;
        const vk::SurfaceKHR &surface;
        const QueueFamilyIndices queueFamilyIndices;
        const SwapChainSupportDetails &swapChainSupportDetails;

        vk::SwapchainKHR swapchain;
        vk::Format swapchainImageFormat;
        vk::Extent2D swapchainExtent;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;
        std::vector<vk::Framebuffer> swapchainFramebuffers;


    private:
        void cleanupResources();

        void createImageViews();

        void createSwapchain();

        void initSwapchainBuffers(const vk::RenderPass &renderPass);

        void destroy();
    };
}