//
// Created by sjbar on 31/08/2022.
//

#pragma once

#include <GLFW/glfw3.h>
#include "core/CoreTypes.hpp"

namespace Rehnda::Windowing {
    class Window {
    public:
        explicit Window(Pixels width, Pixels height);
        ~Window();

        bool should_close();
        void poll_events();

    private:
        Owner<GLFWwindow*> window;
        Pixels width;
        Pixels height;
    public:
        const GLFWwindow *getWindow() const;
        const Pixels &getWidth() const;
        const Pixels &getHeight() const;
    };
}