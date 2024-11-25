#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui)
if((NOT IMGUI_INCLUDE_DIRS OR NOT IMGUI_LIBRARIES) AND NOT EXISTS ${IMGUI_DIR})
    FetchContent_Populate(
        imgui
        GIT_TAG docking
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/imgui
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