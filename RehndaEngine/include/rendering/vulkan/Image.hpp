//
// Created by sjbar on 18/11/2022.
//

#pragma once

#include <filesystem>
#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda {
    struct ImageProps {
        uint32_t width;
        uint32_t height;

        vk::Format format;
        vk::ImageTiling tiling;
        vk::ImageUsageFlags imageUsageFlags;
        vk::MemoryPropertyFlags memoryPropertyFlags;
        vk::ImageAspectFlags imageAspectFlags;
    };

    class Image {
    public:
        Image(vkr::Device& device, vkr::PhysicalDevice &physicalDevice, ImageProps imageProps);

        [[nodiscard]]
        const vkr::ImageView & getImageView() const;
        [[nodiscard]]
        const vkr::Image & getImage() const;

        void transitionImageLayout(vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

        static vk::Format findSupportedFormat(const vkr::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    private:
        vkr::Device& device;
        vkr::PhysicalDevice& physicalDevice;

        ImageProps imageProps;

        const vkr::Image image;
        vkr::DeviceMemory imageMemory;
        vkr::ImageView imageView;

        vkr::Image createImage();
        vkr::DeviceMemory createDeviceMemory();
        vkr::ImageView createImageView();

    };

} // Rehnda