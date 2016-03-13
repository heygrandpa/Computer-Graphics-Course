cmake_minimum_required(VERSION 3.3)
project(opengl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -framework OpenGL -framework Cocoa -framework CoreVideo -framework IOKit -lglfw3 -lm -lobjc")

set(SOURCE_FILES_C1 c1/colorChanging.cpp)

add_executable(c1 ${SOURCE_FILES_C1})

set(SOURCE_FILES_C2 c2/test.cpp)

add_executable(c2 ${SOURCE_FILES_C2})