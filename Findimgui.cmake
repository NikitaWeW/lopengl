set(IMGUI_DIR ${GVO_SCRIPT_DIR}/gvo/dependencies/imgui)
if((NOT LOGGER_INCLUDE_DIRS OR NOT LOGGER_LIBRARIES) AND NOT EXISTS  ${GVO_SCRIPT_DIR}/gvo/dependencies/c-logger)
    FetchContent_Populate(
        imgui
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
    add_library(imgui ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})
    target_include_directories(imgui PRIVATE ${IMGUI_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS}) 
    set(IMGUI_LIBRARIES imgui CACHE STRING "path to imgui libraries")
endif()