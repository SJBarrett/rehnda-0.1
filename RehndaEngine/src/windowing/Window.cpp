//
// Created by sjbar on 31/08/2022.
//
#include <GLFW/glfw3.h>

#include <memory>

#include "Windowing/Window.hpp"

namespace Rehnda::Windowing {
    Window::Window(Pixels width, Pixels height) : width(width), height(height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width.get(), height.get(), "Rehnda", nullptr, nullptr);
        vulkanRenderer = std::make_unique<VulkanRenderer>(window);
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void Window::pollEvents() {
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

    const VulkanRenderer *Window::getVulkanRenderer() const {
        return vulkanRenderer.get();
    }

    void Window::render() {
        vulkanRenderer->drawFrame();
    }

    void Window::waitIdle() {
        vulkanRenderer->waitForDeviceIdle();
    }
}