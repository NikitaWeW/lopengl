set(IMGUI_DIR ${GVO_SCRIPT_DIR}/gvo/dependencies/imgui)
if((NOT IMGUI_INCLUDE_DIRS OR NOT IMGUI_LIBRARIES) AND NOT EXISTS  ${GVO_SCRIPT_DIR}/gvo/dependencies/imgui)
    FetchContent_Populate(
        imgui
        GIT_TAG docking
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        SOURCE_DIR ${IMGUI_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/imgui
    )
endif()
if(NOT DEFINED IMGUI_INCLUDE_DIRS)
    set(IMGUI_INCLUDE_DIRS ${IMGUI_DIR} ${IMGUI_DIR}/backends CACHE STRING "path to imgui include dirs")
endif()
if(NOT DEFINED IMGUI_LIBRARIES)
    file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/imgui*.cpp")
    foreach(IMGUI_BACKEND_SOURCE ${IMGUI_BACKEND_SOURCES})
        list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/${IMGUI_BACKEND_SOURCE})
    endforeach()
    
    add_library(imgui ${IMGUI_SOURCES})
    target_include_directories(imgui PRIVATE ${IMGUI_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}) 
    set(IMGUI_LIBRARIES imgui CACHE STRING "path to imgui libraries")
endif()