//
// Created by sjbar on 12/11/2022.
//

#pragma once

#include "core/RehndaMath.hpp"

namespace Rehnda {
    // Buffer structures need to have memory aligned according to the spec https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
    // mat4 needs to be 16 byte aligned (can be done using alignas(x)
    struct MVPTransforms {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
}