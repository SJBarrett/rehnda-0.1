//
// Created by sjbar on 31/08/2022.
//

#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "core/CoreTypes.hpp"
#include "rendering/vulkan/VkManager.hpp"

namespace Rehnda::Windowing {
    class Window {
    public:
        explicit Window(Pixels width, Pixels height);
        ~Window();

        bool shouldClose() const;
        void pollEvents();
    private:
        Owner<GLFWwindow*> window;
        Pixels width;
        Pixels height;

        std::unique_ptr<VkManager> vkManager;
    public:
        [[nodiscard]]
        const VkManager* getVkManager() const;

    public:
        [[nodiscard]]
        const GLFWwindow *getWindow() const;

        [[nodiscard]]
        const Pixels &getWidth() const;

        [[nodiscard]]
        const Pixels &getHeight() const;
    };
}