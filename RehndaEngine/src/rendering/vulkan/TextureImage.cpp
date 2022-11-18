//
// Created by sjbar on 18/11/2022.
//

#include "rendering/vulkan/TextureImage.hpp"
#include "rendering/vulkan/BufferHelper.hpp"
#include "rendering/vulkan/SingleTimeCommand.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

namespace Rehnda {

    TextureImage::TextureImage(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vkr::Queue &queue, vkr::CommandPool &commandPool,
                               const std::filesystem::path &pathToTexture) :
            device(device),
            physicalDevice(physicalDevice),
            pixelData(loadImage(pathToTexture)),
            textureImage(createTextureImage()),
            textureImageMemory(createDeviceMemory()),
            textureImageView(createImageView()){
        BufferHelper::CreateBufferAndAssignMemoryProps stagingBufferProps{
                .size = imageSize,
                .bufferUsage = vk::BufferUsageFlagBits::eTransferSrc,
                .requiredMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent
        };
        auto [stagingBuffer, stagingBufferMemory] = BufferHelper::createBuffer(
                device,
                physicalDevice,
                stagingBufferProps
        );

        void *data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixelData, static_cast<size_t>(imageSize));
        stagingBufferMemory.unmapMemory();
        stbi_image_free(pixelData);

        // Wait for image to be ready to transfer to, starting state doesn't matter
        transitionImageLayout(queue, commandPool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        // copy to the image from the staging buffer now that the destination is ready
        copyBufferToImage(stagingBuffer, queue, commandPool);
        // Wait for image to be ready to be read in a fragment shader
        transitionImageLayout(queue, commandPool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    void TextureImage::copyBufferToImage(vkr::Buffer &stagingBuffer, vkr::Queue &queue, vkr::CommandPool &commandPool) const {
        SingleTimeCommand singleTimeCommand{device, queue, commandPool};
        vk::BufferImageCopy region{
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {
                        .width = textureWidth,
                        .height = textureHeight,
                        .depth = 1,
                }
        };
        singleTimeCommand.commandBuffer.copyBufferToImage(*stagingBuffer, *textureImage, vk::ImageLayout::eTransferDstOptimal, region);
    }

    void
    TextureImage::transitionImageLayout(vkr::Queue &queue, vkr::CommandPool &commandPool, vk::ImageLayout oldLayout,
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
                .image = *textureImage,
                .subresourceRange = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
        };
        singleTimeCommand.commandBuffer.pipelineBarrier(sourceStage, destStage, vk::DependencyFlags{}, nullptr, nullptr,
                                                        imageMemoryBarrier);
    }

    vkr::Image TextureImage::createTextureImage() {
        vk::ImageCreateInfo imageCreateInfo{
                .imageType = vk::ImageType::e2D,
                .format = vk::Format::eR8G8B8A8Srgb,
                .extent = {
                        .width = static_cast<uint32_t>(textureWidth),
                        .height = static_cast<uint32_t>(textureHeight),
                        .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = vk::ImageTiling::eOptimal,
                .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                .sharingMode = vk::SharingMode::eExclusive,
                .initialLayout = vk::ImageLayout::eUndefined,
        };
        return {device, imageCreateInfo};
    }

    vkr::ImageView TextureImage::createImageView() {
        vk::ImageViewCreateInfo imageViewCreateInfo{
                .image = *textureImage,
                .viewType = vk::ImageViewType::e2D,
                .format = vk::Format::eR8G8B8A8Srgb,
                .subresourceRange = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                }
        };
        return {device, imageViewCreateInfo};
    }

    void *TextureImage::loadImage(const std::filesystem::path &pathToTexture) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(pathToTexture.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        textureWidth = static_cast<uint32_t>(texWidth);
        textureHeight = static_cast<uint32_t>(texHeight);
        numChannels = 4;
        imageSize = textureWidth * textureHeight * numChannels;
        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }
        return pixels;
    }

    vkr::DeviceMemory TextureImage::createDeviceMemory() {
        vk::MemoryRequirements imageMemoryRequirements = textureImage.getMemoryRequirements();

        vk::MemoryAllocateInfo memoryAllocateInfo{
                .allocationSize = imageSize,
                .memoryTypeIndex = BufferHelper::findMemoryType(physicalDevice, imageMemoryRequirements.memoryTypeBits,
                                                                vk::MemoryPropertyFlagBits::eDeviceLocal)
        };
        vkr::DeviceMemory memory{device, memoryAllocateInfo};
        textureImage.bindMemory(*memory, 0);
        return memory;
    }

    vkr::ImageView &TextureImage::getImageView() {
        return textureImageView;
    }
} // Rehnda