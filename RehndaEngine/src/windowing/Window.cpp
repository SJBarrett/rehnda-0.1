//
// Created by sjbar on 31/08/2022.
//
#include <GLFW/glfw3.h>

#include "Windowing/Window.hpp"
#include "rendering/vulkan/VkInstanceHelpers.hpp"
#include "rendering/vulkan/VkDebugHelpers.hpp"

namespace Rehnda::Windowing {
    Window::Window(Pixels width, Pixels height) : width(width), height(height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width.get(), height.get(), "Rehnda", nullptr, nullptr);
#ifdef NDEBUG
        enableValidationLayers = false;
#else
        enableValidationLayers = true;
#endif

        if (enableValidationLayers) {
            vkInstance = VkInstanceHelpers::build_vulkan_instance({"VK_LAYER_KHRONOS_validation"});
            VkDebugHelpers::setup_debug_messenger(vkInstance, &debugMessenger, nullptr);
        } else {
            vkInstance = VkInstanceHelpers::build_vulkan_instance({});
        }
    }

    Window::~Window() {
        if (enableValidationLayers) {
            VkDebugHelpers::destroy_debug_messenger(vkInstance, debugMessenger);
        }
        glfwDestroyWindow(window);
        glfwTerminate();

        vkInstance.destroy();
    }

    bool Window::should_close() const {
        return glfwWindowShouldClose(window);
    }

    void Window::poll_events() {
        glfwPollEvents();
    }

    const GLFWwindow *Window::get_window() const {
        return window;
    }

    const Pixels &Window::get_width() const {
        return width;
    }

    const Pixels &Window::get_height() const {
        return height;
    }
}