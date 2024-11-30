#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

if((NOT OPENAL_INCLUDE_DIRS OR NOT OPENAL_LIBRARIES) AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/OpenAL)
    FetchContent_Populate(
        openal
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/OpenAL
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/OpenAL
    )
endif()

if(NOT OPENAL_INCLUDE_DIRS)
    set(OPENAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/OpenAL/include/ CACHE STRING "OpenAL include dirs")
endif()
if(NOT OPENAL_LIBRARIES OR OPENAL_SET_BY_SCRIPT)
    enable_language(C)
    set(ALSOFT_UTILS OFF)
    set(ALSOFT_EXAMPLES OFF)
    set(ALSOFT_TESTS OFF)
    set(ALSOFT_INSTALL_UTILS OFF)
    set(ALSOFT_INSTALL_EXAMPLES OFF)
    set(ALSOFT_INSTALL_UTILS OFF)
    if(BUILD_SHARED_LIBS)
        set(LIBTYPE SHARED)
    else()
        set(LIBTYPE STATIC)
    endif()

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/OpenAL ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/OpenAL)
    set(OPENAL_LIBRARIES OpenAL CACHE STRING "OpenAL libraries")
    set(OPENAL_SET_BY_SCRIPT ON CACHE BOOL "indicates, if <name>_LIBRARIES was not set by the user")
endif()