#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "core/CoreTypes.hpp"
#include "windowing/Window.hpp"

using namespace Rehnda;

int main() {
    std::cout << "Hello, W2orld!" << std::endl;
    Windowing::Window window(Pixels(600), Pixels(800));

    while (!window.should_close()) {

    }

    return 0;
}
