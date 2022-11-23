//
// Created by sjbar on 22/11/2022.
//

#pragma once

#include "rendering/vulkan/VkTypes.hpp"
#include "Image.hpp"

namespace Rehnda {

    class DepthImage {
    public:
        DepthImage(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vk::Extent2D extent);

        const vkr::ImageView& getImageView() const;

        void resize(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vk::Extent2D extent);

        static vk::Format findDepthFormat(const vkr::PhysicalDevice &physicalDevice);
    private:
        vk::Format depthImageFormat;
        std::unique_ptr<Image> image;

        bool hasStencilComponent();
    };

} // Rehnda