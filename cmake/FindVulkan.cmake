#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

if((NOT VULKAN_INCLUDE_DIRS OR NOT VULKAN_LIBRARIES) AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Headers)
    if(NOT VULKAN_HEADERS_GIT_TAG)
        set(VULKAN_HEADERS_GIT_TAG v1.3.300)
    endif()
    FetchContent_Populate(
        vulkan-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG ${VULKAN_HEADERS_GIT_TAG}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Headers
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers
    )
endif()
# wtf is going on?
if(NOT VULKAN_LIBRARIES OR VULKAN_SET_BY_SCRIPT)
    enable_language(C)
    execute_process(COMMAND ${CMAKE_COMMAND} 
        -S ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Headers 
        -B ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers
        -G ${CMAKE_GENERATOR}
        -D CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    )
    if(NOT ${RESULT} EQUAL 0)
        message(WARNING "failed to configure vulkan headers")
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers --prefix ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers/
        RESULT_VARIABLE RESULT
    )
    if(NOT ${RESULT} EQUAL 0)
        message(WARNING "failed to install vulkan headers")
    endif()
    if(NOT VULKAN_INCLUDE_DIR)
        set(VULKAN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers/include)
    endif()
    set(VULKAN_HEADERS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Headers)
    if(NOT VULKAN_LOADER_GIT_TAG)
        set(VULKAN_LOADER_GIT_TAG v1.3.300)
    endif()
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Loader)
        FetchContent_Populate(
            vulkan-loader
            GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
            GIT_TAG ${VULKAN_LOADER_GIT_TAG}
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Loader/
            BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Loader
        )
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Loader ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/Vulkan-Loader)
    set(VULKAN_LIBRARIES vulkan CACHE STRING "vulkan libraries")
    set(VULKAN_SET_BY_SCRIPT ON CACHE BOOL "indicates, if <name>_LIBRARIES was not set by the user")
endif()

if(NOT VULKAN_INCLUDE_DIRS)
    set(VULKAN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Vulkan-Headers/include/ CACHE STRING "vulkan include dirs")
endif()