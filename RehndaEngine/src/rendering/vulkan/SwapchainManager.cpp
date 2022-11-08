//
// Created by sjbar on 18/09/2022.
//

#include "rendering/vulkan/SwapchainManager.hpp"
#include <limits>
#include <spdlog/spdlog.h>

namespace Rehnda {
    SwapchainManager::SwapchainManager(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice, vk::Device *device,
                                       const vk::SurfaceKHR &surface, QueueFamilyIndices indices) : device(device) {
        createSwapchain(window, physicalDevice, device, surface, indices);
        createImageViews();
    }

    void SwapchainManager::createSwapchain(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice,
                                           const vk::Device *device, const vk::SurfaceKHR &surface,
                                           QueueFamilyIndices &indices) {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physicalDevice, surface);
        const auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
        const auto presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
        const auto extent = chooseSwapExtent(window, swapChainSupportDetails.capabilities);

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;

        // want one more than the minimum so we don't have to wait for the driver to complete operations
        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupportDetails.capabilities.maxImageCount) {
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

        uint32_t queueFamilyIndices[] = {indices.graphicsQueueIndex.value(),
                                         indices.presentQueueIndex.value()};

        if (indices.graphicsQueueIndex.value() != indices.presentQueueIndex.value()) {
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
// if the window is resized the swapchainManager needs to be re-created, and the previous swap chain referenced
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        swapchain = device->createSwapchainKHR(createInfo);
        swapchainImages = device->getSwapchainImagesKHR(swapchain);
    }

    void SwapchainManager::resize(GLFWwindow *window, const vk::PhysicalDevice &physicalDevice,
                                  const vk::SurfaceKHR &surface, QueueFamilyIndices indices, const vk::RenderPass &renderPass) {
        device->waitIdle();
        cleanupResources();
        createSwapchain(window, physicalDevice, device, surface, indices);
        createImageViews();
        initSwapchainBuffers(renderPass);
    }

    void SwapchainManager::createImageViews() {
        swapchainImageViews.resize(swapchainImages.size());

        for (size_t i = 0; i < swapchainImages.size(); i++) {
            vk::ImageViewCreateInfo imageViewCreateInfo{
                    .image = swapchainImages[i],
                    .viewType = vk::ImageViewType::e2D,
                    .format = swapchainImageFormat,
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
            swapchainImageViews[i] = device->createImageView(imageViewCreateInfo);
        }
    }

    SwapChainSupportDetails
    SwapchainManager::querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
        return {
                .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface),
                .formats = physicalDevice.getSurfaceFormatsKHR(surface),
                .presentModes = physicalDevice.getSurfacePresentModesKHR(surface),
        };
    }

    vk::SurfaceFormatKHR
    SwapchainManager::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR
    SwapchainManager::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D
    SwapchainManager::chooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities) {
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

    void SwapchainManager::cleanupResources() {
        // TODO: To support moving between SDR and HDR monitors the renderpass would need to be recreated too
        for (auto framebuffer: swapchainFramebuffers) {
            device->destroyFramebuffer(framebuffer);
        }
        device->destroySwapchainKHR(swapchain);
        for (auto &swapchainImageView: swapchainImageViews) {
            device->destroyImageView(swapchainImageView);
        }
    }

    void SwapchainManager::destroy() {
        cleanupResources();
        destroyed = true;
    }

    vk::Extent2D SwapchainManager::getExtent() const {
        return swapchainExtent;
    }

    vk::Format SwapchainManager::getImageFormat() const {
        return swapchainImageFormat;
    }

    void SwapchainManager::initSwapchainBuffers(const vk::RenderPass &renderPass) {
        swapchainFramebuffers.resize(swapchainImageViews.size());
        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            vk::ImageView attachments[] = {
                    swapchainImageViews[i],
            };

            vk::FramebufferCreateInfo framebufferCreateInfo{
                    // need the renderpass for the framebuffer to be created to be compatible with it
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = swapchainExtent.width,
                    .height = swapchainExtent.height,
                    .layers = 1,
            };

            swapchainFramebuffers[i] = device->createFramebuffer(framebufferCreateInfo);
        }
    }

    vk::Framebuffer SwapchainManager::getSwapchainFramebuffer(size_t bufferIndex) const {
        return swapchainFramebuffers[bufferIndex];
    }

    vk::ResultValue<uint32_t> SwapchainManager::acquireNextImageIndex(vk::Semaphore &imageAvailableSemaphore) {
        return device->acquireNextImageKHR(swapchain, UINT64_MAX, imageAvailableSemaphore);
    }

    PresentResult SwapchainManager::present(const std::vector<vk::Semaphore> &waitSemaphores, vk::Queue &presentQueue,
                                   uint32_t imageIndex) {
        vk::SwapchainKHR swapChains[] = {swapchain};
        vk::PresentInfoKHR presentInfoKhr{
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
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

    SwapchainManager::~SwapchainManager() {
        assert(destroyed);
    }
}