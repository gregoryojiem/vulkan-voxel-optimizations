cmake_minimum_required(VERSION 3.28)
project(vulkan_voxel)

set(CMAKE_CXX_STANDARD 23)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm-1.0.1")

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.4.bin.WIN64/include")
add_library(glfw STATIC IMPORTED
        src/main.cpp
        src/game/world/Block.cpp
        src/game/world/Block.h
        src/game/World.cpp
        src/game/World.h
)
set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a")

add_executable(vulkan_voxel
        src/main.cpp
        src/rendering/WorldGeometry.cpp
        src/rendering/WorldGeometry.h
        src/rendering/camera/Camera.cpp
        src/rendering/camera/Camera.h
        src/rendering/GameRenderer.cpp
        src/rendering/GameRenderer.h
        src/utility/TimeManager.cpp
        src/utility/TimeManager.h
        src/utility/InputHandler.cpp
        src/utility/InputHandler.h
        src/game/World.h
        src/game/World.cpp
        src/game/Block.cpp
        src/game/Block.h
)

target_link_libraries(vulkan_voxel ${Vulkan_LIBRARIES})
target_link_libraries(vulkan_voxel glfw)
