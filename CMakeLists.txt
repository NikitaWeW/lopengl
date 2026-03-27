cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(breakout C CXX)

######################################################################

include(cpm.cmake)

foreach(LOADER GLTF)
    set(ASSIMP_BUILD_${LOADER}_IMPORTER ON)
endforeach()
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

CPMAddPackage("gh:assimp/assimp@6.0.2")
CPMAddPackage("gh:zeux/meshoptimizer@1.0")
CPMAddPackage("gh:glfw/glfw#3.4")
CPMAddPackage("gh:nikitawew/nicecs#c78f357")
CPMAddPackage(URI "gh:gabime/spdlog@1.17.0" FORCE ON)

file(GLOB_RECURSE DEPENDENCIES_SOURCES "dependencies/compile/*")
add_library(dependencies ${DEPENDENCIES_SOURCES})
target_include_directories(dependencies PUBLIC dependencies/include)
target_link_libraries(dependencies INTERFACE assimp meshoptimizer glfw nicecs::ecs spdlog)

######################################################################

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(lopengl ${SOURCES})
target_link_libraries(lopengl PRIVATE dependencies)
target_include_directories(lopengl PRIVATE src)
target_compile_definitions(lopengl PRIVATE GLM_ENABLE_EXPERIMENTAL)
