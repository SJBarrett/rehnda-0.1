//
// Created by sjbar on 16/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include "VkTypes.hpp"


namespace Rehnda {
    struct QueueFamilyIndices {
        std::optional<GraphicsQueueIndex> graphicsQueueIndex;

        bool requiredFamiliesFound() {
            return graphicsQueueIndex.has_value();
        }
    };

    class VkManager {
    public:
        explicit VkManager();
        ~VkManager();
    private:
        vk::Instance instance;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;

        vk::Queue graphicsQueue;

        bool enableValidationLayers;
        vk::DebugUtilsMessengerEXT debugMessenger;

    private:
        [[nodiscard]]
        vk::PhysicalDevice pickPhysicalDevice() const;

        static vk::Device createDevice(const vk::PhysicalDevice& physicalDevice);
        static QueueFamilyIndices findQueues(const vk::PhysicalDevice& device);
        static int rateDeviceSuitability(const vk::PhysicalDevice& device);
    };
}
