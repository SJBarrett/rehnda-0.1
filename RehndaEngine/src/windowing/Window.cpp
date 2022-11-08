//
// Created by sjbar on 31/08/2022.
//
#include <GLFW/glfw3.h>

#include <memory>
#include <spdlog/spdlog.h>

#include "Windowing/Window.hpp"

namespace Rehnda::Windowing {
    Window::Window(Pixels width, Pixels height) : width(width), height(height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width.get(), height.get(), "Rehnda", nullptr, nullptr);
        vulkanRenderer = std::make_unique<VulkanRenderer>(window);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int, int) {
            reinterpret_cast<Window*>(glfwGetWindowUserPointer(w))->resize();
        });
        glfwSetWindowUserPointer(window, this);
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
        if (isWindowMinimized()) {
            return;
        }
        vulkanRenderer->drawFrame();
    }

    void Window::waitIdle() {
        vulkanRenderer->waitForDeviceIdle();
    }

    void Window::resize() {
        vulkanRenderer->resize();
    }

    bool Window::isWindowMinimized() const {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) {
            // Don't render while the window is minimized
            return true;
        }
        return false;
    }
}