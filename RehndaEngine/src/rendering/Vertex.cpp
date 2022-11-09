//
// Created by sjbar on 10/11/2022.
//

#include "rendering/Vertex.hpp"

namespace Rehnda {
    vk::VertexInputBindingDescription Vertex::getBindingDescription() {
        return {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex // move to the next data entry after each vertex
        };
    }

    std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0] = {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat, // 2 float values
                .offset = offsetof(Vertex, pos)
        };

        attributeDescriptions[1] = {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat, // 3 float values
                .offset = offsetof(Vertex, color)
        };

        return attributeDescriptions;
    }
}


