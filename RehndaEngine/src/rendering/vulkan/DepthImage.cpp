//
// Created by sjbar on 22/11/2022.
//

#include "rendering/vulkan/DepthImage.hpp"

namespace Rehnda {
    DepthImage::DepthImage(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vk::Extent2D extent) :
            depthImageFormat(findDepthFormat(physicalDevice)),
            image(std::make_unique<Image>(device, physicalDevice, ImageProps{
                    .width = extent.width,
                    .height = extent.height,
                    .format = depthImageFormat,
                    .tiling = vk::ImageTiling::eOptimal,
                    .imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                    .memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
                    .imageAspectFlags = vk::ImageAspectFlagBits::eDepth,
            })) {

    }

    vk::Format DepthImage::findDepthFormat(const vkr::PhysicalDevice &physicalDevice) {
        return Image::findSupportedFormat(
                physicalDevice,
                {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                vk::ImageTiling::eOptimal,
                vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }

    bool DepthImage::hasStencilComponent() {
        return depthImageFormat == vk::Format::eD32SfloatS8Uint || depthImageFormat == vk::Format::eD24UnormS8Uint;
    }

    const vkr::ImageView &DepthImage::getImageView() const {
        return image->getImageView();
    }

    void DepthImage::resize(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vk::Extent2D extent) {
        device.waitIdle();
        image.reset();
        image = std::make_unique<Image>(device, physicalDevice, ImageProps{
                .width = extent.width,
                .height = extent.height,
                .format = depthImageFormat,
                .tiling = vk::ImageTiling::eOptimal,
                .imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                .memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
                .imageAspectFlags = vk::ImageAspectFlagBits::eDepth,
        });
    }


} // Rehnda