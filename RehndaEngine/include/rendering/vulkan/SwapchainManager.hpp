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

    class SwapchainManager {
    public:
        ~SwapchainManager();
        void initSwapchain(GLFWwindow *window, const vk::PhysicalDevice& physicalDevice, const vk::Device* device, const vk::SurfaceKHR& surface);
        void destroy();

        vk::Extent2D getExtent() const;
        vk::Format getImageFormat() const;

        static SwapChainSupportDetails
        querySwapChainSupport(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface);
    private:
        bool initialized = false;

        vk::SwapchainKHR swapchain;
        vk::Format swapchainImageFormat;
        vk::Extent2D swapchainExtent;
        std::vector <vk::Image> swapchainImages;
        std::vector <vk::ImageView> swapchainImageViews;

        NonOwner<const vk::Device*> owningDevice;

    private:
        void createImageViews();

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

        vk::Extent2D chooseSwapExtent(GLFWwindow *window, const vk::SurfaceCapabilitiesKHR &capabilities);
    };
}