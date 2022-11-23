//
// Created by sjbar on 18/11/2022.
//

#include "rendering/vulkan/Image.hpp"
#include "rendering/vulkan/BufferHelper.hpp"
#include "rendering/vulkan/SingleTimeCommand.hpp"

namespace Rehnda {

    Image::Image(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, const ImageProps imageProps) :
            device(device),
            physicalDevice(physicalDevice),
            imageProps(imageProps),
            image(createImage()),
            imageMemory(createDeviceMemory()),
            imageView(createImageView()){
    }

    void
    Image::transitionImageLayout(vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout,
                                        vk::ImageLayout newLayout) const {
        SingleTimeCommand singleTimeCommand{device, queue, commandPool};
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destStage;
        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destStage = vk::PipelineStageFlagBits::eTransfer;
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            dstAccessMask = vk::AccessFlagBits::eShaderRead;
            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destStage = vk::PipelineStageFlagBits::eEarlyFragmentTests; // reading of the depth buffer happens in EarlyFragmentTests, writing happens in LateFragmentTests
        } else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vk::ImageMemoryBarrier imageMemoryBarrier{
                .srcAccessMask = srcAccessMask,
                .dstAccessMask = dstAccessMask,
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = *image,
                .subresourceRange = {
                        .aspectMask = imageProps.imageAspectFlags,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
        };
        singleTimeCommand.commandBuffer.pipelineBarrier(sourceStage, destStage, vk::DependencyFlags{}, nullptr, nullptr,
                                                        imageMemoryBarrier);
    }

    vkr::Image Image::createImage() {
        vk::ImageCreateInfo imageCreateInfo{
                .imageType = vk::ImageType::e2D,
                .format = imageProps.format,
                .extent = {
                        .width = static_cast<uint32_t>(imageProps.width),
                        .height = static_cast<uint32_t>(imageProps.height),
                        .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = imageProps.tiling,
                .usage = imageProps.imageUsageFlags,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined,
        };
        return {device, imageCreateInfo};
    }

    vkr::ImageView Image::createImageView() {
        vk::ImageViewCreateInfo imageViewCreateInfo{
                .image = *image,
                .viewType = vk::ImageViewType::e2D,
                .format = imageProps.format,
                .subresourceRange = {
                        .aspectMask = imageProps.imageAspectFlags,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                }
        };
        return {device, imageViewCreateInfo};
    }

    vkr::DeviceMemory Image::createDeviceMemory() {
        vk::MemoryRequirements imageMemoryRequirements = image.getMemoryRequirements();

        vk::MemoryAllocateInfo memoryAllocateInfo{
                .allocationSize = imageMemoryRequirements.size,
                .memoryTypeIndex = BufferHelper::findMemoryType(physicalDevice, imageMemoryRequirements.memoryTypeBits,
                                                                imageProps.memoryPropertyFlags)
        };
        vkr::DeviceMemory memory{device, memoryAllocateInfo};
        image.bindMemory(*memory, 0);
        return memory;
    }

    const vkr::ImageView & Image::getImageView() const {
        return imageView;
    }

    const vkr::Image & Image::getImage() const {
        return image;
    }

    vk::Format
    Image::findSupportedFormat(const vkr::PhysicalDevice &physicalDevice, const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                               vk::FormatFeatureFlags features) {
        for (const auto format : candidates) {
            vk::FormatProperties properties = physicalDevice.getFormatProperties(format);
            if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("Failed to find supported format");
    }
} // Rehnda