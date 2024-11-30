#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

if((NOT LOGGER_INCLUDE_DIRS OR NOT LOGGER_LIBRARIES) AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/logger)
    FetchContent_Populate(
        logger
        GIT_REPOSITORY https://github.com/yksz/c-logger.git
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/logger
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/logger
    )
endif()

if(NOT LOGGER_INCLUDE_DIRS)
    set(LOGGER_INCLUDE_DIRS${CMAKE_CURRENT_SOURCE_DIR}/dependencies/logger/src CACHE STRING "logger include dirs")
endif()
if(NOT LOGGER_LIBRARIES OR LOGGER_SET_BY_SCRIPT)
    enable_language(C)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/logger "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/logger")
    if(BUILD_SHARED_LIBS)
        set(LOGGER_LIBRARIES logger CACHE STRING "logger libraries")
    else()
        set(LOGGER_LIBRARIES logger_static CACHE STRING "logger libraries")
    endif()
    set(LOGGER_SET_BY_SCRIPT ON CACHE BOOL "indicates, if <name>_LIBRARIES was not set by the user")
endif()