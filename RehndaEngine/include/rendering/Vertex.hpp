//
// Created by sjbar on 10/11/2022.
//

#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace Rehnda {
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        // describing the chunk as a single unit of data
        static vk::VertexInputBindingDescription getBindingDescription();

        // describing what's inside a vertex
        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
    };
}