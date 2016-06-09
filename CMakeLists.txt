cmake_minimum_required(VERSION 3.3)
project(opengl)

# OpenGL Package
find_package(OpenGL REQUIRED)

# Link Libraries
set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw3
        GLEW
        SOIL
)
link_libraries(${ALL_LIBS})

# Compile Define Options
add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

# Include Current Directory
include_directories(. /usr/local/include)
link_directories(/usr/local/lib)

# C++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")

# Shader Helper Sources
set(SHADER_HELPER_SOURCES
        utils/shader.cpp
        utils/shader.h
)

# Shader GLSL Directory
set(SHADER_DIR shaders)
# Resource Directory
set(RESOURCE_DIR resources)

# Copy GLSLs to Build Dir
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR})
file(GLOB SHADER_SOURCES ${CMAKE_SOURCE_DIR}/${SHADER_DIR}/*)
add_custom_target(COPY_GLSL)
foreach(glsl ${SHADER_SOURCES})
    add_custom_command(TARGET COPY_GLSL PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            copy ${glsl} ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}/)
endforeach()

# Copy Resources to Build Dir
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIR})
file(GLOB RESOURCES ${CMAKE_SOURCE_DIR}/${RESOURCE_DIR}/*)
add_custom_target(COPY_RESOURCES)
foreach(res ${RESOURCES})
    add_custom_command(TARGET COPY_RESOURCES PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            copy ${res} ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_DIR}/)
endforeach()

# c1
set(SOURCE_FILES_C1 c1/colorChanging.cpp)
add_executable(c1 ${SOURCE_FILES_C1})

# c2
set(SOURCE_FILES_C2 c2/polygonRotateAndScale.cpp)
add_executable(c2 ${SOURCE_FILES_C2})

# c3
set(SOURCE_FILES_C3
        c3/animation.cpp
        ${SHADER_HELPER_SOURCES}
)
add_executable(c3 ${SOURCE_FILES_C3})
add_dependencies(c3 COPY_GLSL) # GLSL needed

# c5
set(SOURCE_FILES_C5
        c5/solarSystem.cpp
        ${SHADER_HELPER_SOURCES})
add_executable(c5 ${SOURCE_FILES_C5})
add_dependencies(c5 COPY_GLSL) # GLSL needed
add_dependencies(c5 COPY_RESOURCES) # GLSL needed

# c6
set(SOURCE_FILES_C6
        c6/objviewer.cpp
        c6/HalfEdge.cpp
        ${SHADER_HELPER_SOURCES})
add_executable(c6 ${SOURCE_FILES_C6})
add_dependencies(c6 COPY_GLSL) # GLSL needed
add_dependencies(c6 COPY_RESOURCES) # GLSL needed

# c7
set(SOURCE_FILES_C7
        c7/bspline.cpp
        ${SHADER_HELPER_SOURCES})
add_executable(c7 ${SOURCE_FILES_C7})
add_dependencies(c7 COPY_GLSL) # GLSL needed

# c7.surface
set(SOURCE_FILES_C7.2
        c7/b_surface.cpp
        ${SHADER_HELPER_SOURCES})
add_executable(c7.surface ${SOURCE_FILES_C7.2})
add_dependencies(c7.surface COPY_GLSL) # GLSL needed

# c8
set(SOURCE_FILES_C8
        c8/main.cpp
        c8/aabb.cpp
        c8/abvh.cpp
        c8/basic_builder.cpp
        c8/basic_envmap.cpp
        c8/basic_rasterizer.cpp
        c8/basic_shader.cpp
        c8/block.cpp
        c8/cone.cpp
        c8/cylinder.cpp
        c8/list.cpp
        c8/params.cpp
        c8/plugins.cpp
        c8/point.cpp
        c8/ppm_image.cpp
        c8/quad.cpp
        c8/quartic.cpp
        c8/scene.cpp
        c8/sphere.cpp
        c8/torus.cpp
        c8/transform.cpp
        c8/triangle.cpp
        c8/util.cpp)
add_executable(c8 ${SOURCE_FILES_C8})
add_dependencies(c8 COPY_RESOURCES) # GLSL needed
