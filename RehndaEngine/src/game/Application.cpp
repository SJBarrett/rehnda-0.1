//
// Created by sjbar on 1/09/2022.
//

#include "game/Application.hpp"
#include <core/CoreTypes.hpp>

namespace Rehnda {
    Application::Application() : window(Windowing::Window(Pixels(800), Pixels(600))) {}

    void Application::run() {
        while (!window.shouldClose()) {
            window.pollEvents();
            window.render();
        }
        window.waitIdle();
    }
}