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
    private:
        void transitionImageLayout(vkr::Device &device, vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vkr::Image &image) const;
    };

} // Rehnda