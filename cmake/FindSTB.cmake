#Copyright (c) 2024 Nikita Martynau (https://opensource.org/license/mit)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(FetchContent)

if(NOT STB_INCLUDE_DIRS)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/STB)
        FetchContent_Populate(
            STB
            GIT_REPOSITORY https://github.com/nothings/stb.git
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb
            BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/build/stb
        )
    endif()
    set(STB_INSTALL_INCLUDE_DIRS OFF)
    install(DIRECTORY SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb DESTINATION include FILES_MATCHING PATTERN "*.h")
    set(STB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/ CACHE STRING "STB include dirs")
endif()