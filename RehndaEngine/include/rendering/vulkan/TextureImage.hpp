//
// Created by sjbar on 18/11/2022.
//

#pragma once

#include <filesystem>
#include "rendering/vulkan/VkTypes.hpp"
#include "Image.hpp"

namespace Rehnda {

    class TextureImage {
    public:
        TextureImage(vkr::Device& device, vkr::PhysicalDevice &physicalDevice, vkr::Queue& queue, vkr::CommandPool &commandPool, const std::filesystem::path& pathToTexture);

        [[nodiscard]]
        const vkr::ImageView &getImageView() const;

    private:
        vkr::Device& device;

        void* pixelData;
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t numChannels;
        vk::DeviceSize imageSize;

        Image image;

        void* loadImage(const std::filesystem::path &pathToTexture);

        void copyBufferToImage(vkr::Buffer& stagingBuffer, vkr::Queue &queue, vkr::CommandPool &commandPool) const;
    };

} // Rehnda