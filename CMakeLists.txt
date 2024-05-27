cmake_minimum_required(VERSION 3.28)
project(vulkan_voxel)

set(CMAKE_CXX_STANDARD 23)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm-1.0.1")

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.4.bin.WIN64/include")
add_library(glfw STATIC IMPORTED
        src/main.cpp)
set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a")

add_executable(vulkan_voxel
        src/main.cpp
        src/rendering/vertex.cpp
        src/rendering/vertex.h
        src/rendering/camera.cpp
        src/rendering/camera.h
        src/rendering/GameRenderer.cpp
        src/rendering/GameRenderer.h
        src/rendering/TimeManager.cpp
        src/rendering/TimeManager.h
        src/input/InputHandler.cpp
        src/input/InputHandler.h
)

target_link_libraries(vulkan_voxel ${Vulkan_LIBRARIES})
target_link_libraries(vulkan_voxel glfw)
