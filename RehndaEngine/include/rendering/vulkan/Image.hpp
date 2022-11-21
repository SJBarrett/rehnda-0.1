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
        vk::DeviceSize size;

        vk::Format format;
        vk::ImageTiling tiling;
        vk::ImageUsageFlags imageUsageFlags;
        vk::MemoryPropertyFlags memoryPropertyFlags;
    };

    class Image {
    public:
        Image(vkr::Device& device, vkr::PhysicalDevice &physicalDevice, const ImageProps imageProps);

        vkr::ImageView &getImageView();
        [[nodiscard]]
        const vkr::Image & getImage() const;

        void transitionImageLayout(vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

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