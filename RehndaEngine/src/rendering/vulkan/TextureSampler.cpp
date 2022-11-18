//
// Created by sjbar on 18/11/2022.
//

#include "rendering/vulkan/TextureSampler.hpp"

namespace Rehnda {
    TextureSampler::TextureSampler(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, TextureSamplerProps textureSamplerProps) :
            sampler(createTextureSampler(device, physicalDevice, textureSamplerProps)) {

    }

    vkr::Sampler TextureSampler::createTextureSampler(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, TextureSamplerProps& textureSamplerProps) {
        vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();

        vk::SamplerCreateInfo samplerCreateInfo{
                // how to interpolate texels that are magnified or minified
                .magFilter = textureSamplerProps.magMinFilter,
                .minFilter = textureSamplerProps.magMinFilter,
                // mipmap mode
                .mipmapMode = vk::SamplerMipmapMode::eLinear,
                // Addressing mode such as repeat, mirror or clamp on each axis
                .addressModeU = textureSamplerProps.samplerAddressModeUVW,
                .addressModeV = textureSamplerProps.samplerAddressModeUVW,
                .addressModeW = textureSamplerProps.samplerAddressModeUVW,
                // mipmapping settings
                .mipLodBias = 0.0f,
                // whether to use anisotropic filtering, low maxAnisotropy is better performance
                .anisotropyEnable = true,
                .maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy,
                // if a comparison is enabled, texels will first be compared to a value, and the result of that comparison is used in filtering ops
                .compareEnable = false,
                .compareOp = vk::CompareOp::eAlways,
                // mipmapping settings
                .minLod = 0.0f,
                .maxLod = 0.0f,
                // color to return when sampling beyond the image
                .borderColor = vk::BorderColor::eFloatOpaqueBlack,
                // unnormalizedCoordinates true means -> [0, texWidth), false means -> [0, 1)
                .unnormalizedCoordinates = false,
        };
        return {device, samplerCreateInfo};
    }

    vk::Sampler TextureSampler::operator*() {
        return *sampler;
    }
}