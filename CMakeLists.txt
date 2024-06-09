cmake_minimum_required(VERSION 3.24.1)
project(vulkan_voxel)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

set(FREETYPE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/freetype-2.13.2/objs/freetype.lib")
set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/freetype-2.13.2/include")
find_package(Freetype REQUIRED)

include_directories(
        ${Vulkan_INCLUDE_DIRS}
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm-1.0.1"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/misc"
        ${FREETYPE_INCLUDE_DIRS}
)

add_subdirectory("dependencies/glfw-3.4")

set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a")

add_executable(vulkan_voxel
        src/main.cpp
        src/rendering/Vertex.cpp
        src/rendering/Vertex.h
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
        src/game/ChunkManager.cpp
        src/game/ChunkManager.h
        src/utility/GraphicsUtil.cpp
        src/utility/GraphicsUtil.h
        src/rendering/VertexPool.cpp
        src/rendering/VertexPool.h
        src/rendering/TextRenderer.cpp
        src/rendering/TextRenderer.h
        src/utility/TextUtil.cpp
        src/utility/TextUtil.h
        src/rendering/VulkanUtil.cpp
        src/rendering/VulkanUtil.h
        src/rendering/VulkanDebugger.cpp
        src/rendering/VulkanDebugger.h
        src/rendering/Swapchain.cpp
        src/rendering/Swapchain.h
)

target_link_libraries(vulkan_voxel
        ${FREETYPE_LIBRARIES}
        ${Vulkan_LIBRARIES}
        glfw
)