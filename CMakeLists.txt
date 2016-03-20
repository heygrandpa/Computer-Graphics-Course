cmake_minimum_required(VERSION 3.3)
project(opengl)

# OpenGL Package
find_package(OpenGL REQUIRED)

# Link Libraries
set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw3
        GLEW
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
include_directories(.)

# C++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")

# Shader Helper Sources
set(SHADER_HELPER_SOURCES
        utils/shader.cpp
        utils/shader.h
)

# Shader GLSL Directory
set(SHADER_DIR shaders)

# Copy GLSLs to Build Dir
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR})
file(GLOB SHADER_SOURCES ${CMAKE_SOURCE_DIR}/${SHADER_DIR}/*)
add_custom_target(COPY_GLSL)
foreach(glsl ${SHADER_SOURCES})
    add_custom_command(TARGET COPY_GLSL PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            copy ${glsl} ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}/)
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

