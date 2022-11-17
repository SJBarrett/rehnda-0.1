//
// Created by sjbar on 19/09/2022.
//

#pragma once



#include "rendering/vulkan/VkTypes.hpp"
#include "core/CoreTypes.hpp"
#include "SwapchainManager.hpp"
#include "StagedBuffer.hpp"
#include "rendering/RenderableMesh.hpp"
#include "WritableDirectBuffer.hpp"

namespace Rehnda {
    class GraphicsPipeline {
    public:
        explicit GraphicsPipeline(vkr::Device &device, vk::Format imageFormat, RenderableMesh &renderableMesh,
                                  vkr::DescriptorSetLayout &descriptorSetLayout);

        void recordCommandBuffer(vkr::CommandBuffer &commandBuffer, vkr::Framebuffer &targetFramebuffer,
                                 vkr::DescriptorSet &currentDescriptorSet, vk::Extent2D extent);

        [[nodiscard]]
        const vkr::RenderPass& getRenderPass() const;

    private:
        vkr::Device &device;

        vkr::RenderPass renderPass;
        vkr::PipelineLayout pipelineLayout;
        vkr::Pipeline pipeline;

        // temp variables
        RenderableMesh &mesh;

    private:
        vkr::ShaderModule createShaderModule(const std::vector<char> &code);

        vkr::RenderPass createRenderPass(vk::Format imageFormat);
        vkr::PipelineLayout createPipelineLayout(vkr::DescriptorSetLayout &descriptorSetLayout);
        vkr::Pipeline createPipeline();
    };
}
