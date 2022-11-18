//
// Created by sjbar on 18/11/2022.
//

#pragma once

#include <filesystem>
#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda {

    class TextureImage {
    public:
        TextureImage(vkr::Device& device, vkr::PhysicalDevice &physicalDevice, vkr::Queue& queue, vkr::CommandPool &commandPool, const std::filesystem::path& pathToTexture);

        vkr::ImageView &getImageView();

    private:
        vkr::Device& device;
        vkr::PhysicalDevice& physicalDevice;

        void* pixelData;
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t numChannels;
        vk::DeviceSize imageSize;

        vkr::Image textureImage;
        vkr::DeviceMemory textureImageMemory;
        vkr::ImageView textureImageView;

        void* loadImage(const std::filesystem::path &pathToTexture);
        vkr::Image createTextureImage();
        vkr::DeviceMemory createDeviceMemory();
        vkr::ImageView createImageView();

        void transitionImageLayout(vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

        void copyBufferToImage(vkr::Buffer& stagingBuffer, vkr::Queue &queue, vkr::CommandPool &commandPool) const;
    };

} // Rehnda