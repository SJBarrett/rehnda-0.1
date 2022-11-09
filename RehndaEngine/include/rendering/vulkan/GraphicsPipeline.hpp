//
// Created by sjbar on 19/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include "core/CoreTypes.hpp"
#include "SwapchainManager.hpp"
#include "VertexBuffer.hpp"

namespace Rehnda {
    class GraphicsPipeline {
    public:
        explicit GraphicsPipeline(vk::Device& device, vk::PhysicalDevice& physicalDevice, SwapchainManager* swapchainManager);
        void destroy();

        void recordCommandBuffer(vk::CommandBuffer& commandBuffer, uint32_t imageIndex);

        [[nodiscard]]
        vk::RenderPass getRenderPass() const;

    private:
        vk::Device& device;
        NonOwner<SwapchainManager*> swapchainManager;

        vk::PipelineLayout pipelineLayout;
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;

        // temp variables
        VertexBuffer vertexBuffer;

    private:
        vk::ShaderModule createShaderModule(const std::vector<char>& code);

        vk::RenderPass createRenderPass();

    };
}
