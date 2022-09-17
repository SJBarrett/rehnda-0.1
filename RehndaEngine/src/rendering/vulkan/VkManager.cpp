//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/VkManager.hpp"


#include <map>
#include <set>
#include <limits>
#include <algorithm>

#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/VkInstanceHelpers.hpp"

const std::vector<const char *> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace Rehnda {
    VkManager::VkManager(GLFWwindow *window) {
#ifdef NDEBUG
        enableValidationLayers = false;
#else
        enableValidationLayers = true;
#endif
        if (enableValidationLayers) {
            instance = VkInstanceHelpers::buildVulkanInstance({"VK_LAYER_KHRONOS_validation"});
            VkDebugHelpers::setupDebugMessenger(instance, &debugMessenger, nullptr);
        } else {
            instance = VkInstanceHelpers::buildVulkanInstance({});
        }
        if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr,
                                    reinterpret_cast<VkSurfaceKHR *>(&surface)) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }

        physicalDevice = pickPhysicalDevice();


        device = createDevice(physicalDevice, surface);
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
        graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueueIndex.value().get(), 0);
        presentQueue = device.getQueue(queueFamilyIndices.presentQueueIndex.value().get(), 0);

        swapchain = createSwapchain(window);
    }

    vk::SwapchainKHR VkManager::createSwapchain(GLFWwindow* window) {
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

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
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

        return device.createSwapchainKHR(createInfo);
    }

    VkManager::~VkManager() {
        if (enableValidationLayers) {
            VkDebugHelpers::destroy_debug_messenger(instance, debugMessenger);
        }
        device.destroySwapchainKHR(swapchain);
        device.destroy();
        instance.destroySurfaceKHR(surface);
        instance.destroy();
    }


    vk::PhysicalDevice VkManager::pickPhysicalDevice() const {
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.size() == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::multimap<int, vk::PhysicalDevice> candidates;

        for (const auto &device: physicalDevices) {
            int score = rateDeviceSuitability(device, surface);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            return candidates.rbegin()->second;
        } else {
            throw std::runtime_error("Failed to find suitable GPU");
        }
    }

    vk::Device VkManager::createDevice(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surfaceKhr) {
        const auto queueFamilyIndices = findQueueFamilies(physicalDevice, surfaceKhr);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
                queueFamilyIndices.graphicsQueueIndex.value().get(),
                queueFamilyIndices.presentQueueIndex.value().get()
        };

        float queuePriority = 1.0f;

        for (uint32_t queueFamilyIndex: uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority,
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }


        vk::PhysicalDeviceFeatures physicalDeviceFeatures;

        vk::DeviceCreateInfo deviceCreateInfo{
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
                .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        return physicalDevice.createDevice(deviceCreateInfo);
    }

    QueueFamilyIndices
    VkManager::findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr) {
        QueueFamilyIndices indices;

        const auto queueFamilyProperties = device.getQueueFamilyProperties();

        int i = 0;
        for (const auto &queueFamily: queueFamilyProperties) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsQueueIndex = GraphicsQueueIndex(i);
            }

            if (device.getSurfaceSupportKHR(i, surfaceKhr)) {
                indices.presentQueueIndex = PresentQueueIndex(i);
            }

            if (indices.requiredFamiliesFound()) {
                break;
            }
            i++;
        }


        return indices;
    }

    int VkManager::rateDeviceSuitability(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr) {
        const auto deviceProperties = device.getProperties();
        const auto deviceFeatures = device.getFeatures();
        int score = 1;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) {
            return 0;
        }

        const QueueFamilyIndices indices = findQueueFamilies(device, surfaceKhr);
        if (!indices.graphicsQueueIndex.has_value()) {
            // we require a graphics queue to render
            return 0;
        }

        if (!areRequiredExtensionsSupported(device)) {
            return 0;
        }

        const auto swapChainSupportDetails = querySwapChainSupport(device, surfaceKhr);
        if (swapChainSupportDetails.formats.empty() || swapChainSupportDetails.presentModes.empty()) {
            return 0;
        }

        return score;
    }

    bool VkManager::areRequiredExtensionsSupported(const vk::PhysicalDevice &device) {
        const auto availableExtensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails
    VkManager::querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
        return {
                .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface),
                .formats = physicalDevice.getSurfaceFormatsKHR(surface),
                .presentModes = physicalDevice.getSurfacePresentModesKHR(surface),
        };
    }

    vk::SurfaceFormatKHR VkManager::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR VkManager::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D VkManager::chooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR &capabilities) {
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
}