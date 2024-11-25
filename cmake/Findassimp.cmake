#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

if((NOT ASSIMP_INCLUDE_DIRS OR NOT ASSIMP_LIBRARIES) AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp)
    FetchContent_Populate(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/assimp
    )
endif()

if(NOT ASSIMP_INCLUDE_DIRS)
    set(ASSIMP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp/include CACHE STRING "assimp include dirs")
endif()

if(NOT ASSIMP_LIBRARIES)
    enable_language(C)
    if(NOT ASSIMP_BUILD_TESTS)    
        set(ASSIMP_BUILD_TESTS OFF)
    endif()
    if(NOT ASSIMP_WARNINGS_AS_ERRORS)
        set(ASSIMP_WARNINGS_AS_ERRORS OFF)
    endif()
    if(NOT ASSIMP_BUILD_ASSIMP_VIEW)
        set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
    endif()

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/assimp)
    set(ASSIMP_LIBRARIES assimp CACHE STRING "assimp libraries")
    if(NOT ASSIMP_INCLUDE_DIRS) # assimp generates some headers for some fucking reason. 
        set(ASSIMP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/assimp/include;${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp/include CACHE STRING "assimp include dirs")
    endif()
endif()