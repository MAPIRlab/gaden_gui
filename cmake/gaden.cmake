find_package(gaden QUIET)
if(NOT gaden)
    FetchContent_Declare(
        gaden
        GIT_REPOSITORY git@github.com:MAPIRlab/gaden_core.git
        GIT_TAG main
    )
    
    FetchContent_GetProperties(gaden)
    if(NOT gaden_POPULATED)
        FetchContent_Populate(gaden)
        add_subdirectory(${gaden_SOURCE_DIR})
    endif()
endif()
