# This line will be changed if reconfiguration is required: Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris ligula. 

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

project(learning-opengl C CXX)
set(GLAD_API "gl:core=4.3")
set(IMGUI_BACKEND_SOURCES imgui_impl_opengl3.cpp imgui_impl_glfw.cpp)
set(DEPS OpenGL GLFW Logger GLM imgui STB assimp)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(main ${SOURCES})
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include/)
target_include_directories(main PRIVATE "src")

if(NOT ASSIMP_INCLUDE_DIRS)
    set(ASSIMP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/include)
endif()
if(WIN32) # we dont want to spend time building big library
    if(NOT ASSIMP_LIBRARIES)
        set(ASSIMP_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/win/lib/libassimp.dll.a)
        add_custom_command(
            TARGET main
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/win/bin/libassimp-5.dll ${CMAKE_BINARY_DIR}
        )
    endif()
elseif(UNIX AND NOT APPLE) # linux
    if(NOT ASSIMP_LIBRARIES)
        set(ASSIMP_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/linux/libassimp.so)
        add_custom_command(
            TARGET main
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/linux/libassimp.so      ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

message(STATUS "Dependencies:")
foreach(DEP_NAME ${DEPS})
    string(TOUPPER ${DEP_NAME} DEP_NAME_CAP)
    find_package(${DEP_NAME} REQUIRED)
    target_link_libraries(main PRIVATE ${${DEP_NAME_CAP}_LIBRARIES})
    target_include_directories(main PRIVATE ${${DEP_NAME_CAP}_INCLUDE_DIRS})
    message(STATUS "\t${DEP_NAME}:")
    message(STATUS "\t\tlib:     \"${${DEP_NAME_CAP}_LIBRARIES}\"")
    message(STATUS "\t\tinclude: \"${${DEP_NAME_CAP}_INCLUDE_DIRS}\"")
endforeach()

message(STATUS "Sources: ")
foreach(SOURCE ${SOURCES})
    message(STATUS "\t${SOURCE}")
endforeach()