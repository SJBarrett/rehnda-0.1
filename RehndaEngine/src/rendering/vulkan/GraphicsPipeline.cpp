//
// Created by sjbar on 19/09/2022.
//

#include "rendering/vulkan/GraphicsPipeline.hpp"
#include "core/FileUtils.hpp"
#include "rendering/Vertex.hpp"

namespace Rehnda {
    const std::vector<Vertex> vertices = {
            {{0.0f,  -0.5f}, {1.0f, 1.0f, 1.0f}},
            {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
    };

    /**
     * Summary of what's going on:
     *  - Shader stages: the shader modules that define the functionality of the programmable stages of the graphics pipeline
     *  - Fixed-function state: all of the structures that define the fixed-function stages of the pipeline, like input assembly, rasterizer, viewport and color blending
     *  - Pipeline layout: the uniform and push values referenced by the shader that can be updated at draw time
     *  - Render pass: the attachments referenced by the pipeline stages and their usage
     * @param device
     * @param swapchainManager
     */
    GraphicsPipeline::GraphicsPipeline(vk::Device &device, vk::PhysicalDevice &physicalDevice,
                                       vk::CommandPool &memoryCommandPool, vk::Queue& graphicsQueue, SwapchainManager *swapchainManager) :
            device(device), swapchainManager(swapchainManager),
            vertexBuffer(device, physicalDevice, memoryCommandPool, graphicsQueue, vertices) {
        renderPass = createRenderPass();
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

        const auto vertBindingDescription = Vertex::getBindingDescription();
        const auto vertAttributeDescriptions = Vertex::getAttributeDescriptions();

        // describe the format of the vertex data to be passed in
        vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
                // bindings specify spacing between data and whether per-vertex or instance
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &vertBindingDescription,
                // attributes describe the type of attributes passed, which binding to load them from and at what offset
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertAttributeDescriptions.size()),
                .pVertexAttributeDescriptions = vertAttributeDescriptions.data(),
        };

        // input assembly describes what kind of geometry will be drawn from vertices, and if primitive restart should be enabled
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
                .topology = vk::PrimitiveTopology::eTriangleList,
                .primitiveRestartEnable = VK_FALSE,
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

        vk::PipelineColorBlendStateCreateInfo colorBlending{
                .logicOpEnable = VK_FALSE,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachmentState,
        };

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
        };
        pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
                // --- SHADER STAGE DESCRIPTIONS ---
                .stageCount = 2,
                .pStages = shaderStages,
                // --- FIXED FUNCTION STAGE DESCRIPTION ---
                .pVertexInputState = &vertexInputCreateInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = nullptr,
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicStateCreateInfo,
                // --- PIPELINE LAYOUT ---
                .layout = pipelineLayout,
                // --- RENDER PASS ---
                .renderPass = renderPass,
                .subpass = 0,
                // --- OPTIONAL BASE PIPELINE,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = -1,
        };

        pipeline = device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo).value;

        // shader modules can be destroyed after the spir-v is compiled and linked to machine code during pipeline creation
        device.destroyShaderModule(vertShaderModule);
        device.destroyShaderModule(fragShaderModule);

        swapchainManager->initSwapchainBuffers(renderPass);
    }

    vk::ShaderModule GraphicsPipeline::createShaderModule(const std::vector<char> &code) {
        vk::ShaderModuleCreateInfo createInfo{
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t *>(code.data()),
        };
        return device.createShaderModule(createInfo);
    }

    void GraphicsPipeline::destroy() {
        vertexBuffer.destroy();
        device.destroyPipeline(pipeline);
        device.destroyPipelineLayout(pipelineLayout);
        device.destroyRenderPass(renderPass);
    }

    vk::RenderPass GraphicsPipeline::createRenderPass() {
        vk::AttachmentDescription colorAttachment{
                .format = swapchainManager->getImageFormat(),
                .samples = vk::SampleCountFlagBits::e1,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                // since we are clearing at the start, the initial layout doesn't matter
                .initialLayout = vk::ImageLayout::eUndefined,
                // since we are rendering to the swapchainManager we use PresentSrcKHR
                .finalLayout = vk::ImageLayout::ePresentSrcKHR,
        };
        vk::AttachmentReference colorAttachmentRef{
                .attachment = 0,
                .layout = vk::ImageLayout::eColorAttachmentOptimal,
        };

        vk::SubpassDescription subpass{
                .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                .colorAttachmentCount = 1,
                // the index of the attachment in the below array is what is referened in the shader (e.g. "layout(location = 0) out vec4 outColor;")
                .pColorAttachments = &colorAttachmentRef,
        };

        vk::SubpassDependency subpassDependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL, // this refers to the implicit subpass before the render pass
                .dstSubpass = 0, // this refers to our subpass, the first and only one
                // we need to wait for the swap chain to finish reading from the image before we access it
                .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .srcAccessMask = vk::AccessFlagBits::eNone,
                .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        };

        vk::RenderPassCreateInfo renderPassCreateInfo{
                .attachmentCount = 1,
                .pAttachments = &colorAttachment,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &subpassDependency,
        };

        return device.createRenderPass(renderPassCreateInfo);
    }


    void GraphicsPipeline::recordCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) {
        vk::CommandBufferBeginInfo beginInfo{};
        commandBuffer.begin(beginInfo); // this implicitly resets the vertexBuffer

        vk::ClearValue clearColor{
                .color = {
                        .float32 = {{0.0f, 0.f, 0.f, 1.f}}
                }
        };

        vk::RenderPassBeginInfo renderPassBeginInfo{
                .renderPass = renderPass,
                .framebuffer = swapchainManager->getSwapchainFramebuffer(imageIndex),
                .renderArea = {
                        .offset = {0, 0},
                        .extent = swapchainManager->getExtent(),
                },
                .clearValueCount = 1,
                .pClearValues = &clearColor,
        };

        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        vk::Viewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(swapchainManager->getExtent().width),
                .height = static_cast<float>(swapchainManager->getExtent().height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
        };
        commandBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissor{
                .offset = {0, 0},
                .extent = swapchainManager->getExtent(),
        };
        commandBuffer.setScissor(0, 1, &scissor);

        vk::Buffer vertexBuffers[] = {vertexBuffer.getBuffer()};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        // vertex count, instance count, first vertex, first instance
        commandBuffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();
    }

    vk::RenderPass GraphicsPipeline::getRenderPass() const {
        return renderPass;
    }
}