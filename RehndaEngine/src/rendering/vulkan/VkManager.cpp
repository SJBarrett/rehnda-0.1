//
// Created by sjbar on 16/09/2022.
//

#include <map>
#include "rendering/vulkan/VkManager.hpp"
#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/VkInstanceHelpers.hpp"

namespace Rehnda {
    VkManager::VkManager() {
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

        physicalDevice = pickPhysicalDevice();
        device = createDevice(physicalDevice);
        graphicsQueue = device.getQueue(findQueues(physicalDevice).graphicsQueueIndex.value().get(), 0);
    }

    VkManager::~VkManager() {
        if (enableValidationLayers) {
            VkDebugHelpers::destroy_debug_messenger(instance, debugMessenger);
        }
        device.destroy();
        instance.destroy();
    }


    vk::PhysicalDevice VkManager::pickPhysicalDevice() const {
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.size() == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::multimap<int, vk::PhysicalDevice> candidates;

        for (const auto &device: physicalDevices) {
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            return candidates.rbegin()->second;
        } else {
            throw std::runtime_error("Failed to find suitable GPU");
        }
    }

    vk::Device VkManager::createDevice(const vk::PhysicalDevice &physicalDevice) {
        const auto queueFamilyIndices = findQueues(physicalDevice);
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{
                .queueFamilyIndex = queueFamilyIndices.graphicsQueueIndex.value().get(),
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
        };

        vk::PhysicalDeviceFeatures physicalDeviceFeatures;

        vk::DeviceCreateInfo deviceCreateInfo{
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &queueCreateInfo,
                .enabledExtensionCount = 0,
                .pEnabledFeatures = &physicalDeviceFeatures,
        };

        return physicalDevice.createDevice(deviceCreateInfo);
    }

    QueueFamilyIndices VkManager::findQueues(const vk::PhysicalDevice &device) {
        QueueFamilyIndices indices;

        const auto queueFamilyProperties = device.getQueueFamilyProperties();

        int i = 0;
        for (const auto &queueFamily: queueFamilyProperties) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsQueueIndex = GraphicsQueueIndex(i);
            }

            if (indices.requiredFamiliesFound()) {
                break;
            }
            i++;
        }

        return indices;
    }

    int VkManager::rateDeviceSuitability(const vk::PhysicalDevice &device) {
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

        const QueueFamilyIndices indices = findQueues(device);
        if (!indices.graphicsQueueIndex.has_value()) {
            // we require a graphics queue to render
            return 0;
        }

        return score;
    }
}