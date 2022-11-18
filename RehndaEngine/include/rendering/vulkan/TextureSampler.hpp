//
// Created by sjbar on 18/11/2022.
//

#pragma once

#include "rendering/vulkan/VkTypes.hpp"

namespace Rehnda {
    struct TextureSamplerProps {
        vk::Filter magMinFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode samplerAddressModeUVW = vk::SamplerAddressMode::eRepeat;
    };
    class TextureSampler {
    public:
        TextureSampler(vkr::Device& device, vkr::PhysicalDevice& physicalDevice, TextureSamplerProps textureSamplerProps);

        vk::Sampler operator*();
    private:
        vkr::Sampler sampler;

        vkr::Sampler createTextureSampler(vkr::Device &device, vkr::PhysicalDevice &physicalDevice, TextureSamplerProps& textureSamplerProps);
    };
}