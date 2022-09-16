//
// Created by sjbar on 31/08/2022.
//

#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "core/CoreTypes.hpp"

namespace Rehnda::Windowing {
    class Window {
    public:
        explicit Window(Pixels width, Pixels height);
        ~Window();

        bool should_close() const;
        void poll_events();


    private:
        Owner<GLFWwindow*> window;
        Pixels width;
        Pixels height;

        vk::Instance vkInstance;

        bool enableValidationLayers = false;
        vk::DebugUtilsMessengerEXT debugMessenger;
    public:
        [[nodiscard]]
        const GLFWwindow *get_window() const;

        [[nodiscard]]
        const Pixels &get_width() const;

        [[nodiscard]]
        const Pixels &get_height() const;
    };
}