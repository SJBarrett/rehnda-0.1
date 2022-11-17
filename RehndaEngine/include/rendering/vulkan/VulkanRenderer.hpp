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

        void drawFrame();

        void resize();

        void waitForDeviceIdle();

    private:
        NonOwner<GLFWwindow*> window;

        vkr::Context context;
        vkr::Instance instance;
        vkr::DebugUtilsMessengerEXT debugMessenger;
        vkr::SurfaceKHR surface;
        vkr::PhysicalDevice physicalDevice;
        QueueFamilyIndices queueFamilyIndices;
        vkr::Device device;

        std::unique_ptr<FrameCoordinator> frameCoordinator;

        bool enableValidationLayers;

    private:
        vkr::PhysicalDevice pickPhysicalDevice();

        vkr::Device createDevice();

        vkr::SurfaceKHR createSurface();
        QueueFamilyIndices findQueueFamilies();


    private:
        static int rateDeviceSuitability(GLFWwindow* window, const vkr::PhysicalDevice &device, const vkr::SurfaceKHR &surfaceKhr);


        static bool areRequiredExtensionsSupported(const vkr::PhysicalDevice &device, const std::vector<const char *>& requiredExtensions);
    };
}
