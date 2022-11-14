//
// Created by sjbar on 20/09/2022.
//

#pragma once

#include "rendering/vulkan/VkTypes.hpp"
#include <GLFW/glfw3.h>
#include <optional>
#include "FrameCoordinator.hpp"

namespace Rehnda {
    class VulkanRenderer {
    public:
        explicit VulkanRenderer(GLFWwindow *window);
        ~VulkanRenderer();

        void drawFrame();

        void resize();

        void waitForDeviceIdle();

    private:
        NonOwner<GLFWwindow*> window;

        vkr::Context context;
        vkr::Instance instance;
        vkr::DebugUtilsMessengerEXT debugMessenger;
        vkr::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        QueueFamilyIndices queueFamilyIndices;
        vk::Device device;

        std::unique_ptr<FrameCoordinator> frameCoordinator;




        bool enableValidationLayers;

    private:
        vk::PhysicalDevice pickPhysicalDevice();

        vk::Device createDevice();

        vkr::SurfaceKHR createSurface();
        QueueFamilyIndices findQueueFamilies();


    private:
        static int rateDeviceSuitability(GLFWwindow* window, const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr);


        static bool areRequiredExtensionsSupported(const vk::PhysicalDevice &device, const std::vector<const char *>& requiredExtensions);
    };
}
