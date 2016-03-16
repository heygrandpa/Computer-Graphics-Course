# Color Changing

First OpenGL application for testing development environment.

System: Mac OS X 10.10 (OpenGL installed by default)

## Setup Environment 

``` shell
brew install glfw3   
brew install glm
```

## Compile
CMakeLists.txt   
``` cmake
cmake_minimum_required(VERSION 3.3)
project(opengl)

find_package(OpenGL REQUIRED)

link_libraries(glfw3 ${OPENGL_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")

set(SOURCE_FILES_C1 c1/colorChanging.cpp)
add_executable(c1 ${SOURCE_FILES_C1})

set(SOURCE_FILES_C2 c2/polygonRotateAndScale.cpp)
add_executable(c2 ${SOURCE_FILES_C2})

...
```
