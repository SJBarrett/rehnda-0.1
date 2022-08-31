//
// Created by sjbar on 31/08/2022.
//
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <core/Logger.hpp>

#include "Windowing/Window.hpp"

namespace Rehnda::Windowing {
    Window::Window(Pixels width, Pixels height) : width(width), height(height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width.get(), height.get(), "Rehnda", nullptr, nullptr);

        const std::vector<vk::ExtensionProperties> a = vk::enumerateInstanceExtensionProperties();
        Log::info("Num extensions: {}", a.size());
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

    const GLFWwindow *Window::getWindow() const {
        return window;
    }

    const Pixels &Window::getWidth() const {
        return width;
    }

    const Pixels &Window::getHeight() const {
        return height;
    }
}