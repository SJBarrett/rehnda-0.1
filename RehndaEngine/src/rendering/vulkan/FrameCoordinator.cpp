//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/FrameCoordinator.hpp"

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <memory>

#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/SwapchainManager.hpp"
#include "rendering/MVPTransforms.hpp"

namespace Rehnda {
    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    FrameCoordinator::FrameCoordinator(GLFWwindow *window, vkr::Device &device, vkr::PhysicalDevice &physicalDevice,
                                       vkr::SurfaceKHR &surface,
                                       QueueFamilyIndices queueFamilyIndices) :
            device(device),
            physicalDevice(physicalDevice),
            queueFamilyIndices(queueFamilyIndices),
            graphicsQueue(device.getQueue(queueFamilyIndices.graphicsQueueIndex.value(), 0)),
            presentQueue(device.getQueue(queueFamilyIndices.presentQueueIndex.value(), 0)),
            graphicsCommandPool(createCommandPool(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)),
            memoryCommandPool(createCommandPool(vk::CommandPoolCreateFlagBits::eTransient)),
            commandBuffers(createCommandBuffers()),
            imageAvailableSemaphores(createSemaphores(MAX_FRAMES_IN_FLIGHT)),
            renderFinishedSemaphores(createSemaphores(MAX_FRAMES_IN_FLIGHT)),
            inFlightFences(createFences(MAX_FRAMES_IN_FLIGHT)),
            uboBuffers(createUbos()),
            descriptorSetLayout(createDescriptorSetLayout()),
            descriptorPool(createDescriptorPool()),
            descriptorSets(createDescriptorSets()) {

        mesh = std::make_unique<RenderableMesh>(
                DeviceContext{.device = device, .physicalDevice=physicalDevice, .memoryCommandPool = memoryCommandPool, .graphicsQueue=graphicsQueue},
                vertices, indices);
        SwapChainSupportDetails swapChainSupportDetails(window, physicalDevice, surface);
        graphicsPipeline = std::make_unique<GraphicsPipeline>(device,
                                                              swapChainSupportDetails.chooseSwapSurfaceFormat().format,
                                                              *mesh, descriptorSetLayout);
        swapchainManager = std::make_unique<SwapchainManager>(device, surface, queueFamilyIndices,
                                                              graphicsPipeline->getRenderPass(),
                                                              swapChainSupportDetails);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DescriptorBufferInfo bufferInfo{
                    .buffer = *uboBuffers[i].getBuffer(),
                    .offset = 0,
                    .range = sizeof(MVPTransforms)
            };
            vk::WriteDescriptorSet descriptorWrite{
                    .dstSet = *descriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &bufferInfo,
                    .pTexelBufferView = nullptr
            };
            device.updateDescriptorSets(descriptorWrite, nullptr);
        }
    }

    vkr::CommandPool FrameCoordinator::createCommandPool(vk::CommandPoolCreateFlags commandPoolCreateFlags) {
        vk::CommandPoolCreateInfo poolCreateInfo{
                // since we are recording a command buffer every frame we want to be able to reset and rerecord, hence ResetCommandBuffer
                .flags = commandPoolCreateFlags,
                .queueFamilyIndex = queueFamilyIndices.graphicsQueueIndex.value(),
        };
        return {device, poolCreateInfo};
    }

    vkr::CommandBuffers FrameCoordinator::createCommandBuffers() {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
                .commandPool = *graphicsCommandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        };
        return {device, commandBufferAllocateInfo};
    }

    /**
     * Basics of drawing a frame:
     * 1. Wait for previous frame to finish
     * 2. Acquire an image from the swap chain
     * 3. Record a commabd bnuffer which draws the scene
     * 4. Submit the recorded command buffer
     * 5. Present the swap chain image
     */
    DrawFrameResult FrameCoordinator::drawFrame() {
        // waiting for fences would fail if we exit this method early and reset fences immediately before new work is submitted
        const auto waitResult = device.waitForFences({*inFlightFences[currentFrame]}, VK_TRUE, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);

        const auto [result, nextImageIndex] = swapchainManager->acquireNextImageIndex(
                imageAvailableSemaphores[currentFrame]);
        if (result == vk::Result::eErrorOutOfDateKHR || framebufferResized) {
            framebufferResized = false;
            swapchainManager->resize(graphicsPipeline->getRenderPass());
            return DrawFrameResult::SWAPCHAIN_OUT_OF_DATE;
        } else if (result != vk::Result::eSuccess &&
                   result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        // reset only once we have submitted work and know we won't exit early due to swapchain out of date
        device.resetFences({*inFlightFences[currentFrame]});

        updateUniformBuffer(currentFrame);

        commandBuffers[currentFrame].reset();
        graphicsPipeline->recordCommandBuffer(commandBuffers[currentFrame],
                                              swapchainManager->getSwapchainFramebuffer(nextImageIndex),
                                              descriptorSets[currentFrame], swapchainManager->getExtent());

        vk::Semaphore waitSemaphores[] = {*imageAvailableSemaphores[currentFrame]};
        std::vector<vk::Semaphore> signalSemaphores{*renderFinishedSemaphores[currentFrame]};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphores,
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &*commandBuffers[currentFrame],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphores.data(),
        };

        graphicsQueue.submit(submitInfo, *inFlightFences[currentFrame]);

        if (swapchainManager->present(signalSemaphores, presentQueue, nextImageIndex) ==
            PresentResult::SWAPCHAIN_OUT_OF_DATE) {
            return DrawFrameResult::SWAPCHAIN_OUT_OF_DATE;
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return DrawFrameResult::SUCCESS;
    }


    vkr::DescriptorSetLayout FrameCoordinator::createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding uboLayoutBinding{
                .binding = 0,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
                .pImmutableSamplers = nullptr
        };

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{
                .bindingCount = 1,
                .pBindings = &uboLayoutBinding
        };
        return {device, layoutCreateInfo};
    }

    std::vector<WritableDirectBuffer> FrameCoordinator::createUbos() {
        vk::DeviceSize bufferSize = sizeof(MVPTransforms);
        MVPTransforms defaultTransform{};
        const WritableDirectBufferProps bufferProps{
                .dataSize = bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                .data = &defaultTransform
        };
        std::vector<WritableDirectBuffer> buffers;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            buffers.emplace_back(device, physicalDevice, bufferProps);
        }
        return buffers;
    }

    void FrameCoordinator::setFramebufferResized() {
        framebufferResized = true;
    }

    void FrameCoordinator::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        // TODO#1 for frequently changing values such as the MVP transforms, push constants are more efficient than UBOs
        MVPTransforms mvpTransforms{};
        mvpTransforms.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0, 0.0, 1.0f));
        mvpTransforms.view = glm::lookAt(glm::vec3(2.0, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                         glm::vec3(0.0, 0.0f, 1.0f));
        mvpTransforms.proj = glm::perspective(glm::radians(45.f), swapchainManager->getExtent().width /
                                                                  (float) swapchainManager->getExtent().height, 0.1f,
                                              10.f);
        // negate the y scaling factor of the projection matrix as GLM was designed for OpenGL where the y clip co-ordinates are inverted
        mvpTransforms.proj[1][1] *= -1;
        uboBuffers[currentImage].writeData(&mvpTransforms);
    }

    vkr::DescriptorPool FrameCoordinator::createDescriptorPool() {
        vk::DescriptorPoolSize poolSize{
                .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
        };
        vk::DescriptorPoolCreateInfo poolCreateInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                .poolSizeCount = 1,
                .pPoolSizes = &poolSize
        };
        return {device, poolCreateInfo};
    }

    vkr::DescriptorSets FrameCoordinator::createDescriptorSets() {
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
                .descriptorPool = *descriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                .pSetLayouts = layouts.data()
        };
        return {device, descriptorSetAllocateInfo};
    }

    std::vector<vkr::Semaphore> FrameCoordinator::createSemaphores(size_t numToCreate) {
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        std::vector<vkr::Semaphore> semaphores;
        for (size_t i = 0; i < numToCreate; i++) {
            semaphores.emplace_back(device, semaphoreCreateInfo);
        }
        return semaphores;
    }

    std::vector<vkr::Fence> FrameCoordinator::createFences(size_t numToCreate) {
        vk::FenceCreateInfo fenceCreateInfo{
                // fence needs to start signaled for the first loop
                .flags = vk::FenceCreateFlagBits::eSignaled,
        };
        std::vector<vkr::Fence> fences;
        for (size_t i = 0; i < numToCreate; i++) {
            fences.emplace_back(device, fenceCreateInfo);
        }
        return fences;
    }
}