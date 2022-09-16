//
// Created by sjbar on 16/09/2022.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "VkTypes.hpp"


namespace Rehnda {
    class VkManager {
    public:
        explicit VkManager();
        ~VkManager();
    private:
        vk::Instance instance;
        vk::PhysicalDevice physicalDevice;

        bool enableValidationLayers;
        vk::DebugUtilsMessengerEXT debugMessenger;

    private:
        [[nodiscard]]
        vk::PhysicalDevice pickPhysicalDevice() const;
        QueueFamilyIndices findQueues(const vk::PhysicalDevice& device) const;
        int rateDeviceSuitability(const vk::PhysicalDevice& device) const;
    };
}
