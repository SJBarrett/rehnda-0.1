//
// Created by sjbar on 20/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
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

        vk::Instance instance;
        vk::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        std::unique_ptr<FrameCoordinator> frameCoordinator;
        std::unique_ptr<SwapchainManager> swapchainManager;
        vk::Device device;
        QueueFamilyIndices queueFamilyIndices;



        bool enableValidationLayers;
        vk::DebugUtilsMessengerEXT debugMessenger;

    private:
        [[nodiscard]]
        vk::PhysicalDevice pickPhysicalDevice() const;

        void createDevice();

        void initSwapchain(GLFWwindow *window);



    private:
        static int rateDeviceSuitability(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr);

        static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surfaceKhr);

        static bool areRequiredExtensionsSupported(const vk::PhysicalDevice &device, const std::vector<const char *>& requiredExtensions);
    };
}
