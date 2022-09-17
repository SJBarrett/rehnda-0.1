//
// Created by sjbar on 16/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VkTypes.hpp"


namespace Rehnda {
    struct QueueFamilyIndices {
        std::optional<GraphicsQueueIndex> graphicsQueueIndex;
        std::optional<PresentQueueIndex> presentQueueIndex;

        [[nodiscard]]
        bool requiredFamiliesFound() const {
            return graphicsQueueIndex.has_value() && presentQueueIndex.has_value();
        }
    };

    class VkManager {
    public:
        explicit VkManager(GLFWwindow* window);
        ~VkManager();
    private:
        vk::Instance instance;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vk::SurfaceKHR surfaceKhr;

        bool enableValidationLayers;
        vk::DebugUtilsMessengerEXT debugMessenger;

    private:
        [[nodiscard]]
        vk::PhysicalDevice pickPhysicalDevice() const;

        static vk::Device createDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surfaceKhr);
        static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR &surfaceKhr);
        static int rateDeviceSuitability(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr);
    };
}
