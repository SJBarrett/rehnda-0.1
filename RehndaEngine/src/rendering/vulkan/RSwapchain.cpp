//
// Created by sjbar on 18/09/2022.
//

#include "rendering/vulkan/RSwapchain.hpp"
#include "rendering/vulkan/VkManager.hpp"
#include <limits>
#include <spdlog/spdlog.h>

namespace Rehnda {
    void RSwapchain::initSwapchain(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice, const vk::Device* device, const vk::SurfaceKHR& surface) {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physicalDevice, surface);
        const auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
        const auto presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
        const auto extent = chooseSwapExtent(window, swapChainSupportDetails.capabilities);

        // want one more than the minimum so we don't have to wait for the driver to complete operations
        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount) {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo = {
                .surface = surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        };

        QueueFamilyIndices indices = VkManager::findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsQueueIndex.value().get(), indices.presentQueueIndex.value().get() };

        if (indices.graphicsQueueIndex.value().get() != indices.presentQueueIndex.value().get()) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        // preTransform can be used to do operations like rotate 90 deg. or mirror before display
        createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        // opaque prevents blending with other windows through the alpha channel
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE; // if our window is partially hidden, we don't care about rendering those hidden values
        // if the window is resized the swapchain needs to be re-created, and the previous swap chain referenced
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        swapchain = device->createSwapchainKHR(createInfo);
        swapchainImages = device->getSwapchainImagesKHR(swapchain);

        owningDevice = device;
        initialized = true;
    }


    SwapChainSupportDetails RSwapchain::querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
        return {
                .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface),
                .formats = physicalDevice.getSurfaceFormatsKHR(surface),
                .presentModes = physicalDevice.getSurfacePresentModesKHR(surface),
        };
    }

    vk::SurfaceFormatKHR RSwapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR RSwapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D RSwapchain::chooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            vk::Extent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    RSwapchain::~RSwapchain() {
#ifdef DEBUG
        if (initialized) {
            SPDLOG_ERROR("RSwapchain should have been destroyed before being deconstructed!");
        }
#endif
    }

    void RSwapchain::destroy() {
        if (owningDevice == nullptr) {
            SPDLOG_WARN("Tried to destroy a swapchain that hasn't been initialized!");
        } else {
            owningDevice->destroySwapchainKHR(swapchain);
        }
    }
}