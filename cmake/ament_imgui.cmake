find_package(imgui_gl QUIET)
if(NOT imgui_gl_FOUND)
    FetchContent_Declare(
        ament_imgui
        GIT_REPOSITORY git@github.com:PepeOjeda/ament_imgui.git
        GIT_TAG master
    )
    FetchContent_GetProperties(ament_imgui)
    if(NOT ament_imgui_POPULATED)
        FetchContent_Populate(ament_imgui)
        add_subdirectory(${ament_imgui_SOURCE_DIR}/imgui_gl)
    endif()
endif()
