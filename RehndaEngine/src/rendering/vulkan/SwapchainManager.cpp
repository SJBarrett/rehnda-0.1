//
// Created by sjbar on 18/09/2022.
//

#include "rendering/vulkan/SwapchainManager.hpp"
#include <limits>

namespace Rehnda {
    SwapchainManager::SwapchainManager(vkr::Device &device, const vkr::SurfaceKHR &surface, QueueFamilyIndices indices,
                                       const vkr::RenderPass &renderPass, const SwapChainSupportDetails &swapChainSupportDetails) :
            device(device),
            surface(surface),
            queueFamilyIndices(
                    indices),
            swapChainSupportDetails(
                    swapChainSupportDetails),
            swapchainSurfaceFormat(swapChainSupportDetails.chooseSwapSurfaceFormat()),
            swapchainExtent(swapChainSupportDetails.chooseSwapExtent()),
            swapchain(createSwapchain()),
            swapchainImageViews(createImageViews()),
            swapchainFramebuffers(createFrameBuffers(renderPass)){

    }

    vkr::SwapchainKHR SwapchainManager::createSwapchain() {
        const auto presentMode = swapChainSupportDetails.chooseSwapPresentMode();

        // want one more than the minimum, so we don't have to wait for the driver to complete operations
        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupportDetails.capabilities.maxImageCount) {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo = {
                .surface = *surface,
                .minImageCount = imageCount,
                .imageFormat = swapchainSurfaceFormat.format,
                .imageColorSpace = swapchainSurfaceFormat.colorSpace,
                .imageExtent = swapchainExtent,
                .imageArrayLayers = 1,
                .imageUsage = vk::ImageUsageFlagBits::eColorAttachment
        };

        uint32_t indicesArray[] = {queueFamilyIndices.graphicsQueueIndex.value(),
                                   queueFamilyIndices.presentQueueIndex.value()};

        if (queueFamilyIndices.graphicsQueueIndex.value() != queueFamilyIndices.presentQueueIndex.value()) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = indicesArray;
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
        // TODO#3 if the window is resized the swapchainManager needs to be re-created, and the previous swap chain referenced
        createInfo.oldSwapchain = VK_NULL_HANDLE;
       return {device, createInfo};
    }

    void SwapchainManager::resize(const vkr::RenderPass &renderPass) {
        device.waitIdle();
        createSwapchain();
        createImageViews();
        createFrameBuffers(renderPass);
    }

    std::vector<vkr::ImageView> SwapchainManager::createImageViews() {
        const std::vector<VkImage> swapchainImages = swapchain.getImages();
        std::vector<vkr::ImageView> imageViews;
        for (size_t i = 0; i < swapchainImages.size(); i++) {
            vk::ImageViewCreateInfo imageViewCreateInfo{
                    .image = swapchainImages[i],
                    .viewType = vk::ImageViewType::e2D,
                    .format = swapchainSurfaceFormat.format,
                    .components = {
                            .r = vk::ComponentSwizzle::eIdentity,
                            .g = vk::ComponentSwizzle::eIdentity,
                            .b = vk::ComponentSwizzle::eIdentity,
                            .a = vk::ComponentSwizzle::eIdentity,
                    },
                    .subresourceRange = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                    },
            };
            imageViews.emplace_back(device, imageViewCreateInfo);
        }
        return imageViews;
    }

    vk::Extent2D SwapchainManager::getExtent() const {
        return swapchainExtent;
    }

    std::vector<vkr::Framebuffer> SwapchainManager::createFrameBuffers(const vkr::RenderPass &renderPass) {
        std::vector<vkr::Framebuffer> buffers;
        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            vk::ImageView attachments[] = {
                    *swapchainImageViews[i],
            };

            vk::FramebufferCreateInfo framebufferCreateInfo{
                    // need the renderpass for the framebuffer to be created to be compatible with it
                    .renderPass = *renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = swapchainExtent.width,
                    .height = swapchainExtent.height,
                    .layers = 1,
            };

            buffers.emplace_back(device, framebufferCreateInfo);
        }
        return buffers;
    }

    vkr::Framebuffer &SwapchainManager::getSwapchainFramebuffer(size_t bufferIndex) {
        return swapchainFramebuffers[bufferIndex];
    }

    std::pair<vk::Result, uint32_t> SwapchainManager::acquireNextImageIndex(vkr::Semaphore &imageAvailableSemaphore) {
        return swapchain.acquireNextImage(UINT64_MAX, *imageAvailableSemaphore);
    }

    PresentResult SwapchainManager::present(const std::vector<vk::Semaphore> &waitSemaphores, vkr::Queue &presentQueue,
                                            uint32_t imageIndex) {
        vk::SwapchainKHR swapChains[] = {*swapchain};
        vk::PresentInfoKHR presentInfoKhr{
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = reinterpret_cast<const vk::Semaphore *>(waitSemaphores.data()),
                .swapchainCount = 1,
                .pSwapchains = swapChains,
                .pImageIndices = &imageIndex,
        };
        const auto presentResult = presentQueue.presentKHR(presentInfoKhr);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR) {
            return PresentResult::SWAPCHAIN_OUT_OF_DATE;
        }
        assert(presentResult == vk::Result::eSuccess);
        return PresentResult::SUCCESS;
    }

    SwapChainSupportDetails::SwapChainSupportDetails(GLFWwindow *window,
                                                     const vkr::PhysicalDevice &physicalDevice,
                                                     const vkr::SurfaceKHR &surface) :
            capabilities(physicalDevice.getSurfaceCapabilitiesKHR(*surface)),
            window(window),
            formats(physicalDevice.getSurfaceFormatsKHR(*surface)),
            presentModes(physicalDevice.getSurfacePresentModesKHR(*surface)) {
    }

    vk::SurfaceFormatKHR SwapChainSupportDetails::chooseSwapSurfaceFormat() const {
        for (const auto &availableFormat: formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return formats[0];
    }

    vk::PresentModeKHR SwapChainSupportDetails::chooseSwapPresentMode() const {
        for (const auto &availablePresentMode: presentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D SwapChainSupportDetails::chooseSwapExtent() const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            vk::Extent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}