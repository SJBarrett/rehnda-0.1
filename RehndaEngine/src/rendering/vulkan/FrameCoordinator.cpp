//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/FrameCoordinator.hpp"


#include <memory>
#include <set>

#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/SwapchainManager.hpp"

namespace Rehnda {

    FrameCoordinator::FrameCoordinator(NonOwner<vk::Device*> device, NonOwner<SwapchainManager*> swapchainManager, QueueFamilyIndices queueFamilyIndices) : device(device), swapchainManager(swapchainManager),
                                                                                                                                                            queueFamilyIndices(queueFamilyIndices) {
        graphicsQueue = device->getQueue(queueFamilyIndices.graphicsQueueIndex.value(), 0);
        presentQueue = device->getQueue(queueFamilyIndices.presentQueueIndex.value(), 0);

        initCommandPool();
        commandBuffer = createCommandBuffer();
        graphicsPipeline = std::make_unique<GraphicsPipeline>(device, swapchainManager);
        createSyncObjects();
    }

    void FrameCoordinator::destroy() {
        device->destroySemaphore(imageAvailableSemaphore);
        device->destroySemaphore(renderFinishedSemaphore);
        device->destroyFence(inFlightFence);
        graphicsPipeline->destroy();
        device->destroyCommandPool(graphicsCommandPool);
        destroyed = true;
    }

    void FrameCoordinator::initCommandPool() {
        vk::CommandPoolCreateInfo poolCreateInfo {
            // since we are recording a command buffer every frame we want to be able to reset and rerecord, hence ResetCommandBuffer
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamilyIndices.graphicsQueueIndex.value(),
        };
        graphicsCommandPool = device->createCommandPool(poolCreateInfo);
    }

    /**
     * Basics of drawing a frame:
     * 1. Wait for previous frame to finish
     * 2. Acquire an image from the swap chain
     * 3. Record a commabd bnuffer which draws the scene
     * 4. Submit the recorded command buffer
     * 5. Present the swap chain image
     */
    void FrameCoordinator::drawFrame() {
        const auto waitResult = device->waitForFences({inFlightFence}, VK_TRUE, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        device->resetFences({inFlightFence});

        uint32_t nextImageIndex = swapchainManager->acquireNextImageIndex(imageAvailableSemaphore);
        commandBuffer.reset();
        graphicsPipeline->recordCommandBuffer(commandBuffer, nextImageIndex);

        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
        std::vector<vk::Semaphore> signalSemaphores { renderFinishedSemaphore };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores.data(),
        };

        const auto submitResult = graphicsQueue.submit(1, &submitInfo, inFlightFence);
        assert(submitResult == vk::Result::eSuccess);
        swapchainManager->present(signalSemaphores, presentQueue, nextImageIndex);
    }

    void FrameCoordinator::createSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreCreateInfo {};
        imageAvailableSemaphore = device->createSemaphore(semaphoreCreateInfo);
        renderFinishedSemaphore = device->createSemaphore(semaphoreCreateInfo);

        vk::FenceCreateInfo fenceCreateInfo {
            // fence needs to start signaled for the first loop
            .flags = vk::FenceCreateFlagBits::eSignaled,
        };
        inFlightFence = device->createFence(fenceCreateInfo);
    }

    vk::CommandBuffer FrameCoordinator::createCommandBuffer() {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
                .commandPool = graphicsCommandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
        };
        return device->allocateCommandBuffers(commandBufferAllocateInfo)[0];
    }

    FrameCoordinator::~FrameCoordinator() {
        assert(destroyed);
    }
}