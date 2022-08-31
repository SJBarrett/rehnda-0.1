set(ENGINE_TARGET_NAME rehnda)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++20 -lws2_32")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
        glm/0.9.9.7
        ms-gsl/4.0.0
        fmt/9.0.0
        spdlog/1.10.0
        magic_enum/0.8.0
        BASIC_SETUP BUILD missing BUILD_TYPE Debug)

set(SOURCE_FILES
        src/main.cpp
        )

add_executable(${ENGINE_TARGET_NAME} ${SOURCE_FILES})
set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${ENGINE_TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${ENGINE_TARGET_NAME}
        ${CONAN_LIBS}
        )
include_directories(include)

add_custom_command(
        TARGET ${ENGINE_TARGET_NAME}
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/shaders)
add_custom_command(
        TARGET ${ENGINE_TARGET_NAME}
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${ENGINE_TARGET_NAME}>/resources)