//
// Created by sjbar on 1/09/2022.
//

#pragma once

#include <windowing/Window.hpp>

namespace Rehnda {
    class Application {
    public:
        Application();
        void run();

    private:
        Windowing::Window window;
    };
}