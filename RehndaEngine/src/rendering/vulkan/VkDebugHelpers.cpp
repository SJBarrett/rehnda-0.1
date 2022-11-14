//
// Created by sjbar on 16/09/2022.
//

#include "rendering/vulkan/VkDebugHelpers.hpp"

#include <spdlog/spdlog.h>

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       [[maybe_unused]] void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        SPDLOG_ERROR("VkDebugError -> {}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        SPDLOG_WARN("VkDebugWarn -> {}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        SPDLOG_DEBUG("VkDebugInfo -> {}", pCallbackData->pMessage);
    } else {
        SPDLOG_DEBUG("VkDebugVerbose -> {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

namespace Rehnda::VkDebugHelpers {
    vkr::DebugUtilsMessengerEXT setupDebugMessenger(vkr::Instance& instance) {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo = build_debug_messenger_create_info();
        return {instance, createInfo};
    }

    vk::DebugUtilsMessengerCreateInfoEXT build_debug_messenger_create_info() {
        return {
                .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                .pfnUserCallback = debug_callback,
                .pUserData = nullptr
        };
    }
}