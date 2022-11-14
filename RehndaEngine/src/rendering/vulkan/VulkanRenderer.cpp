//
// Created by sjbar on 20/09/2022.
//

#include "rendering/vulkan/VulkanRenderer.hpp"

#include <map>
#include <set>

#include "rendering/vulkan/VkInstanceHelpers.hpp"
#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/SwapchainManager.hpp"

const std::vector<const char *> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace Rehnda {
    // TODO#4 Don't include validation layers in release builds
    VulkanRenderer::VulkanRenderer(GLFWwindow *window) :
            window(window),
            instance(VkInstanceHelpers::buildVulkanInstance(context, {"VK_LAYER_KHRONOS_validation"})),
            debugMessenger(VkDebugHelpers::setupDebugMessenger(instance)),
            surface(createSurface()),
            physicalDevice(pickPhysicalDevice()),
            queueFamilyIndices(findQueueFamilies()),
            device(createDevice()) {
        vk::SurfaceKHR surf = *surface;
        frameCoordinator = std::make_unique<FrameCoordinator>(window, device, physicalDevice, surf,
                                                              queueFamilyIndices);
    }

    VulkanRenderer::~VulkanRenderer() {
        frameCoordinator.reset();
        device.destroy();
    }

    vk::PhysicalDevice VulkanRenderer::pickPhysicalDevice() {
        const auto physicalDevices = instance.enumeratePhysicalDevices();
        return *physicalDevices.front();
//        if (physicalDevices.size() == 0) {
//            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
//        }
//
//        std::multimap<int, vk::PhysicalDevice> candidates;
//
//        for (const auto &device: physicalDevices) {
//            int score = rateDeviceSuitability(window, device, *surface);
//            candidates.insert(std::make_pair(score, device));
//        }
//
//        // Check if the best candidate is suitable at all
//        if (candidates.rbegin()->first > 0) {
//            return candidates.rbegin()->second;
//        } else {
//            throw std::runtime_error("Failed to find suitable GPU");
//        }
    }

    int VulkanRenderer::rateDeviceSuitability(GLFWwindow *window, const vk::PhysicalDevice &device,
                                              const vk::SurfaceKHR &surfaceKhr) {
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

//        const QueueFamilyIndices indices = findQueueFamilies(device, surfaceKhr);
//        if (!indices.graphicsQueueIndex.has_value()) {
//            // we require a graphics queue to render
//            return 0;
//        }

        if (!areRequiredExtensionsSupported(device, requiredDeviceExtensions)) {
            return 0;
        }
        SwapChainSupportDetails swapChainSupportDetails{window, device, surfaceKhr};
        if (swapChainSupportDetails.formats.empty() || swapChainSupportDetails.presentModes.empty()) {
            return 0;
        }

        return score;
    }

    QueueFamilyIndices VulkanRenderer::findQueueFamilies() {
        QueueFamilyIndices indices;

        const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        int i = 0;
        for (const auto &queueFamily: queueFamilyProperties) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsQueueIndex = i;
            }

            if (physicalDevice.getSurfaceSupportKHR(i, *surface)) {
                indices.presentQueueIndex = i;
            }

            if (indices.requiredFamiliesFound()) {
                break;
            }
            i++;
        }

        return indices;
    }

    bool VulkanRenderer::areRequiredExtensionsSupported(const vk::PhysicalDevice &device,
                                                        const std::vector<const char *> &requiredExtensions) {
        const auto availableExtensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensionsSet.erase(extension.extensionName);
        }

        return requiredExtensionsSet.empty();
    }

    vk::Device VulkanRenderer::createDevice() {
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
                queueFamilyIndices.graphicsQueueIndex.value(),
                queueFamilyIndices.presentQueueIndex.value()
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

    void VulkanRenderer::waitForDeviceIdle() {
        device.waitIdle();
    }

    void VulkanRenderer::drawFrame() {
        frameCoordinator->drawFrame();
    }

    void VulkanRenderer::resize() {
        frameCoordinator->setFramebufferResized();
    }

    vkr::SurfaceKHR VulkanRenderer::createSurface() {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(static_cast<VkInstance>(*instance), window, nullptr,
                                    &_surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
        return {instance, _surface};
    }
}
