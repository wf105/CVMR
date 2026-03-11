cmake_minimum_required(VERSION 3.10)
project(CameraFrustums)

set(CMAKE_CXX_STANDARD 17)

# Enable Qt's Meta-Object Compiler for signals/slots
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
find_package(OpenCV REQUIRED)

# Include directories
include_directories(
    include
    external/glad/include
    external/glm
)

# Add source files (do NOT add headers here)
add_executable(CameraFrustums
    main.cpp
    source/shader.cpp
    glwidget.cpp
)

# After defining the target, set include dirs and link libraries
target_include_directories(CameraFrustums PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(CameraFrustums
    OpenGL::GL
    glfw
    Qt5::Widgets
    Qt5::OpenGL
    ${OpenCV_LIBS}
)