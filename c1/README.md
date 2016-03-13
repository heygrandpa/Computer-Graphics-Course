# Color Changing

First OpenGL application for testing development environment.

System: Mac OS X 10.10 (OpenGL installed by default)

## Setup Environment 

``` shell
brew install --static homebrew/versions/glfw3   
brew install glm
```

## Compile
CMakeLists.txt   
``` cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -framework OpenGL -framework Cocoa -framework CoreVideo -framework IOKit -lglfw3 -lm -lobjc
```
