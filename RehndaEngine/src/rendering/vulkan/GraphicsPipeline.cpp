//
// Created by sjbar on 19/09/2022.
//

#include "rendering/vulkan/GraphicsPipeline.hpp"
#include "core/FileUtils.hpp"

namespace Rehnda {

    GraphicsPipeline::GraphicsPipeline(const vk::Device *device, SwapchainManager *swapchainManager) : device(device),
                                                                                                       swapchainManager(
                                                                                                               swapchainManager) {
        auto vertShaderCode = FileUtils::readFileAsBytes("shaders/triangle.vert.spv");
        auto fragShaderCode = FileUtils::readFileAsBytes("shaders/triangle.frag.spv");

        auto vertShaderModule = createShaderModule(vertShaderCode);
        auto fragShaderModule = createShaderModule(fragShaderCode);

        // can use pSpecializationInfo to specify shader constants at compile time, which allows the compiler to optimise
        vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = vertShaderModule,
                .pName = "main",
        };

        vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = fragShaderModule,
                .pName = "main",
        };

        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

        // describe the format of the vertex data to be passed in
        vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
                // bindings specify spacing between data and whether per-vertex or instance
                .vertexBindingDescriptionCount = 0,
                .pVertexBindingDescriptions = nullptr,
                // attributes describe the type of attributes passed, which binding to load them from and at what offset
                .vertexAttributeDescriptionCount = 0,
                .pVertexAttributeDescriptions = nullptr,
        };

        // input assembly describes what kind of geometry will be drawn from vertices, and if primitive restart should be enabled
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
                .topology = vk::PrimitiveTopology::eTriangleList,
                .primitiveRestartEnable = VK_FALSE,
        };

        vk::Viewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = (float) swapchainManager->getExtent().width,
                .height = (float) swapchainManager->getExtent().height,
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
        };

        vk::Rect2D scissor = {
                .offset = {0, 0},
                .extent = swapchainManager->getExtent(),
        };

        // dynamic states allow some limited things to be modified without recreated th epipeline
        std::vector<vk::DynamicState> dynamicStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
                .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
                .pDynamicStates = dynamicStates.data(),
        };

        // if the viewport and scissor were not dynamic that would need to be specified in the viewport state create
        vk::PipelineViewportStateCreateInfo viewportState = {
                .viewportCount = 1,
                .scissorCount = 1,
        };

        vk::PipelineRasterizationStateCreateInfo rasterizer = {
                // being true can be useful for shadow maps, other discard outside depth range is best
                .depthClampEnable = VK_FALSE,
                // can be set to true to skip rasterization stage, which means no output to the framebuffer
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eBack,
                .frontFace = vk::FrontFace::eClockwise,
                .depthBiasEnable = VK_FALSE,
                .lineWidth = 1.0f,
        };

        vk::PipelineMultisampleStateCreateInfo multisampling = {
                .rasterizationSamples = vk::SampleCountFlagBits::e1,
                .sampleShadingEnable = VK_FALSE,
        };

        // configure how color is mixed with color already in the framebuffer, either mixing or combine through bitwise operations
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
        };

        vk::PipelineColorBlendStateCreateInfo colorBlending {
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
        };

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
        pipelineLayout = device->createPipelineLayout(pipelineLayoutCreateInfo);


        // shader modules can be destroyed after the spir-v is compiled and linked to machine code during pipeline creation
        device->destroyShaderModule(vertShaderModule);
        device->destroyShaderModule(fragShaderModule);
    }

    vk::ShaderModule GraphicsPipeline::createShaderModule(const std::vector<char> &code) {
        vk::ShaderModuleCreateInfo createInfo{
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t *>(code.data()),
        };
        return device->createShaderModule(createInfo);
    }

    void GraphicsPipeline::destroy() {
        device->destroyPipelineLayout(pipelineLayout);
    }
}