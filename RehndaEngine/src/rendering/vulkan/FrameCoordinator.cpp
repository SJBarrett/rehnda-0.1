//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/FrameCoordinator.hpp"


#include <memory>

#include "rendering/vulkan/VkDebugHelpers.hpp"
#include "rendering/vulkan/SwapchainManager.hpp"

namespace Rehnda {

    FrameCoordinator::FrameCoordinator(NonOwner<vk::Device*> device, NonOwner<SwapchainManager*> swapchainManager, QueueFamilyIndices queueFamilyIndices) : device(device), swapchainManager(swapchainManager),
                                                                                                                                                            queueFamilyIndices(queueFamilyIndices) {
        graphicsQueue = device->getQueue(queueFamilyIndices.graphicsQueueIndex.value(), 0);
        presentQueue = device->getQueue(queueFamilyIndices.presentQueueIndex.value(), 0);

        initCommandPool();
        createCommandBuffers();
        graphicsPipeline = std::make_unique<GraphicsPipeline>(device, swapchainManager);
        createSyncObjects();
    }

    void FrameCoordinator::destroy() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device->destroySemaphore(imageAvailableSemaphores[i]);
            device->destroySemaphore(renderFinishedSemaphores[i]);
            device->destroyFence(inFlightFences[i]);
        }
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

    void FrameCoordinator::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        vk::SemaphoreCreateInfo semaphoreCreateInfo {};
        vk::FenceCreateInfo fenceCreateInfo {
                // fence needs to start signaled for the first loop
                .flags = vk::FenceCreateFlagBits::eSignaled,
        };

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device->createSemaphore(semaphoreCreateInfo);
            renderFinishedSemaphores[i] = device->createSemaphore(semaphoreCreateInfo);
            inFlightFences[i] = device->createFence(fenceCreateInfo);
        }
    }

    void FrameCoordinator::createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo {
                .commandPool = graphicsCommandPool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
        };
        commandBuffers = device->allocateCommandBuffers(commandBufferAllocateInfo);
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
        const auto waitResult = device->waitForFences({inFlightFences[currentFrame]}, VK_TRUE, UINT64_MAX);
        assert(waitResult == vk::Result::eSuccess);
        device->resetFences({inFlightFences[currentFrame]});

        uint32_t nextImageIndex = swapchainManager->acquireNextImageIndex(imageAvailableSemaphores[currentFrame]);
        commandBuffers[currentFrame].reset();
        graphicsPipeline->recordCommandBuffer(commandBuffers[currentFrame], nextImageIndex);

        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        std::vector<vk::Semaphore> signalSemaphores { renderFinishedSemaphores[currentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[currentFrame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores.data(),
        };

        const auto submitResult = graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);
        assert(submitResult == vk::Result::eSuccess);
        swapchainManager->present(signalSemaphores, presentQueue, nextImageIndex);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    FrameCoordinator::~FrameCoordinator() {
        assert(destroyed);
    }
}