//
// Created by sjbar on 31/08/2022.
//
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "Windowing/Window.hpp"

namespace Rehnda::Windowing {
    Window::Window(Pixels width, Pixels height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width.get(), height.get(), "Rehnda", nullptr, nullptr);

        const std::vector<vk::ExtensionProperties> a = vk::enumerateInstanceExtensionProperties();
        spdlog::info("Num extensions: {}", a.size());
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Window::should_close() {
        return glfwWindowShouldClose(window);
    }

    void Window::poll_events() {
        glfwPollEvents();
    }
}