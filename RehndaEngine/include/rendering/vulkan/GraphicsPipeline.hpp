//
// Created by sjbar on 19/09/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include "core/CoreTypes.hpp"
#include "SwapchainManager.hpp"
#include "StagedBuffer.hpp"
#include "rendering/RenderableMesh.hpp"
#include "WritableDirectBuffer.hpp"

namespace Rehnda {
    class GraphicsPipeline {
    public:
        explicit GraphicsPipeline(vk::Device &device, vk::Format imageFormat, RenderableMesh &renderableMesh,
                                  vk::DescriptorSetLayout &descriptorSetLayout);

        ~GraphicsPipeline();

        void recordCommandBuffer(vk::CommandBuffer &commandBuffer, vk::Framebuffer &targetFramebuffer,
                                 vk::DescriptorSet &currentDescriptorSet, vk::Extent2D extent);

        [[nodiscard]]
        vk::RenderPass getRenderPass() const;

    private:
        vk::Device &device;

        vk::PipelineLayout pipelineLayout;
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;

        // temp variables
        RenderableMesh &mesh;

    private:
        vk::ShaderModule createShaderModule(const std::vector<char> &code);

        vk::RenderPass createRenderPass(vk::Format imageFormat);

        void destroy();
    };
}
