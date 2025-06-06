cmake_minimum_required(VERSION 3.11)

project(gaden_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-glldb -O0")

add_subdirectory(third_party/gaden_core)
add_subdirectory(third_party/ament_imgui/ament_imgui)

include_directories(
    src
)

add_executable(gaden_gui
    src/main.cpp
    src/Application.cpp
    src/Project.cpp
)

target_link_libraries(gaden_gui
    gaden
    ament_imgui
)
