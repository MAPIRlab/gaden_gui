cmake_minimum_required(VERSION 3.11)

project(gaden_gui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-glldb -O0")

add_subdirectory(third_party/gaden_core)
add_subdirectory(third_party/ament_imgui/ament_imgui)

include_directories(
    src
    src/glad/include
)

add_executable(gaden_gui
    src/main.cpp
    src/Application.cpp
    src/Project.cpp
    src/Modes/ConfigurationMode.cpp

    src/Visualization/Scene.cpp
    src/glad/src/glad.c
    src/Visualization/Camera.cpp
    )

target_link_libraries(gaden_gui
    gaden
    ament_imgui
)
