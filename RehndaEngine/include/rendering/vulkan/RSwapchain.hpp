//
// Created by sjbar on 18/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "core/CoreTypes.hpp"


namespace Rehnda {
    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class RSwapchain {
    public:
        ~RSwapchain();
        void initSwapchain(GLFWwindow *window, const vk::PhysicalDevice& physicalDevice, const vk::Device* device, const vk::SurfaceKHR& surface);
        void destroy();
        static SwapChainSupportDetails
        querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface);
    private:
        bool initialized = false;
        vk::SwapchainKHR swapchain;
        std::vector <vk::Image> swapchainImages;

        NonOwner<const vk::Device*> owningDevice;

    private:
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

        vk::Extent2D chooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities);
    };
}