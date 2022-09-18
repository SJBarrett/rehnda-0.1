//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/VkManager.hpp"


#include <map>
#include <set>

#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/VkInstanceHelpers.hpp"
#include "rendering/vulkan/RSwapchain.hpp"

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

        swapchain.initSwapchain(window, physicalDevice, &device, surface);
    }

    VkManager::~VkManager() {
        if (enableValidationLayers) {
            VkDebugHelpers::destroy_debug_messenger(instance, debugMessenger);
        }
        swapchain.destroy();
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

        const auto swapChainSupportDetails = RSwapchain::querySwapChainSupport(device, surfaceKhr);
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

}