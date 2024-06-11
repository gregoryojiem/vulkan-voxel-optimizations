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
        src/rendering/misc/Vertex.cpp
        src/rendering/misc/Vertex.h
        src/rendering/misc/Camera.cpp
        src/rendering/misc/Camera.h
        src/rendering/ChunkRenderer.cpp
        src/rendering/ChunkRenderer.h
        src/util/TimeManager.cpp
        src/util/TimeManager.h
        src/util/InputManager.cpp
        src/util/InputManager.h
        src/game/World.h
        src/game/World.cpp
        src/game/Block.cpp
        src/game/Block.h
        src/game/ChunkManager.cpp
        src/game/ChunkManager.h
        src/util/GraphicsUtil.cpp
        src/util/GraphicsUtil.h
        src/rendering/misc/VertexPool.cpp
        src/rendering/misc/VertexPool.h
        src/rendering/TextRenderer.cpp
        src/rendering/TextRenderer.h
        src/util/TextUtil.cpp
        src/util/TextUtil.h
        src/rendering/vulkan/VulkanUtil.cpp
        src/rendering/vulkan/VulkanUtil.h
        src/rendering/vulkan/VulkanDebugger.cpp
        src/rendering/vulkan/VulkanDebugger.h
        src/rendering/vulkan/SwapChain.cpp
        src/rendering/vulkan/SwapChain.h
        src/rendering/vulkan/VulkanBufferUtil.cpp
        src/rendering/vulkan/VulkanBufferUtil.h
        src/rendering/CoreRenderer.cpp
        src/rendering/CoreRenderer.h
        src/rendering/MainRenderer.cpp
        src/rendering/MainRenderer.h
        src/rendering/vulkan/VulkanStructs.h
        src/game/Chunk.cpp
        src/game/Chunk.h
)

target_link_libraries(vulkan_voxel
        ${FREETYPE_LIBRARIES}
        ${Vulkan_LIBRARIES}
        glfw
)