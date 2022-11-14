//
// Created by sjbar on 3/09/2022.
//
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "rendering/vulkan/VkInstanceHelpers.hpp"
#include "rendering/vulkan/VkDebugHelpers.hpp"


namespace Rehnda::VkInstanceHelpers {
    vkr::Instance buildVulkanInstance(vkr::Context &context, std::vector<const char *> validationLayers) {
        vk::ApplicationInfo applicationInfo{
                .pApplicationName = "Rehnda",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "RehndaEngine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_3,
        };

        std::vector<const char *> required_extensions = get_required_extensions(validationLayers);

        const std::vector<vk::ExtensionProperties> &extensions = context.enumerateInstanceExtensionProperties();
        SPDLOG_DEBUG("Enabling extensions:");
        for (const auto &extension: required_extensions) {
            SPDLOG_DEBUG("{}", extension);
        }

        vk::InstanceCreateInfo instanceCreateInfo{
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = 0,
                .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
                .ppEnabledExtensionNames = required_extensions.data(),
        };
        auto debugCreateInfo = VkDebugHelpers::build_debug_messenger_create_info();
        if (!validationLayers.empty()) {
            if (are_validation_layers_supported(context, validationLayers)) {
                instanceCreateInfo.enabledLayerCount = validationLayers.size();
                instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

                instanceCreateInfo.pNext = &debugCreateInfo;
            } else {
                SPDLOG_WARN("Validation layers are desired but not supported");
            }
        }
        return {context, instanceCreateInfo};
    }

    bool are_validation_layers_supported(vkr::Context &context, const std::vector<const char *> &validationLayers) {
        const std::vector<vk::LayerProperties> availableLayers = context.enumerateInstanceLayerProperties();

        for (const auto &layerName: validationLayers) {
            bool layerFound = false;
            for (const auto &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> get_required_extensions(std::vector<const char *> validationLayers) {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // if the khronos validation layer is enabled we need to enable debug utils extensions
        if (std::find(validationLayers.begin(), validationLayers.end(), "VK_LAYER_KHRONOS_validation") != extensions.end()) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }


}
