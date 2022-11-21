//
// Created by sjbar on 18/11/2022.
//

#include "rendering/vulkan/TextureImage.hpp"
#include "rendering/vulkan/BufferHelper.hpp"
#include "rendering/vulkan/SingleTimeCommand.hpp"
#include "rendering/vulkan/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

namespace Rehnda {

    TextureImage::TextureImage(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, vkr::Queue &queue, vkr::CommandPool &commandPool,
                               const std::filesystem::path &pathToTexture) :
            device(device),
            pixelData(loadImage(pathToTexture)),
            image(device, physicalDevice, ImageProps{
                    .width = textureWidth,
                    .height = textureHeight,
                    .size = imageSize,
                    .format = vk::Format::eR8G8B8A8Srgb,
                    .tiling = vk::ImageTiling::eOptimal,
                    .imageUsageFlags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                    .memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
            }) {
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
        image.transitionImageLayout(queue, commandPool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        // copy to the image from the staging buffer now that the destination is ready
        copyBufferToImage(stagingBuffer, queue, commandPool);
        // Wait for image to be ready to be read in a fragment shader
        image.transitionImageLayout(queue, commandPool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
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
        singleTimeCommand.commandBuffer.copyBufferToImage(*stagingBuffer, *image.getImage(), vk::ImageLayout::eTransferDstOptimal, region);
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

    vkr::ImageView &TextureImage::getImageView() {
        return image.getImageView();
    }
} // Rehnda