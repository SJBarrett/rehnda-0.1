set(ENGINE_TARGET_NAME rehnda)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++20 -lws2_32")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)

# FIND VULKAN
find_package(Vulkan REQUIRED)
IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
ENDIF ()
message(STATUS "Vulkan lib path: " ${Vulkan_LIBRARY})
message(STATUS "Vulkan include path: " ${Vulkan_INCLUDE_DIRS})

conan_cmake_run(REQUIRES
        glm/0.9.9.7
        glfw/3.3.7
        ms-gsl/4.0.0
        fmt/9.0.0
        spdlog/1.10.0
        magic_enum/0.8.0
        namedtype/20190324
        BASIC_SETUP BUILD missing BUILD_TYPE Debug)
add_definitions(-DGLFW_INCLUDE_NONE)

set(SOURCE_FILES
        src/main.cpp
        src/windowing/Window.cpp
        src/game/Application.cpp
        src/rendering/vulkan/BufferHelper.cpp
        src/rendering/vulkan/VkInstanceHelpers.cpp
        src/rendering/vulkan/VkDebugHelpers.cpp
        src/rendering/vulkan/FrameCoordinator.cpp
        src/rendering/vulkan/SwapchainManager.cpp
        src/rendering/vulkan/GraphicsPipeline.cpp
        src/rendering/vulkan/StagedBuffer.cpp
        src/rendering/vulkan/WritableDirectBuffer.cpp
        src/rendering/vulkan/VulkanRenderer.cpp
        src/core/FileUtils.cpp
        src/rendering/Vertex.cpp
        src/rendering/RenderableMesh.cpp
        )

add_executable(${ENGINE_TARGET_NAME} ${SOURCE_FILES})
set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${ENGINE_TARGET_NAME}
        ${CONAN_LIBS}
        ${Vulkan_LIBRARY}
        )
include_directories(include ${Vulkan_INCLUDE_DIRS})

add_custom_command(
        TARGET ${ENGINE_TARGET_NAME}
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/shaders)
add_custom_command(
        TARGET ${ENGINE_TARGET_NAME}
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/resources)

# Compile shaders from -> https://gist.github.com/evilactually/a0d191701cb48f157b05be7f74d79396

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else ()
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif ()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        )

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${ENGINE_TARGET_NAME} Shaders)

add_custom_command(TARGET ${ENGINE_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/shaders"
        )