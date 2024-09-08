cmake_minimum_required(VERSION 3.5)
project(learning-opengl C CXX)
set(VULKAN ON)
set(OPENGL ON)

add_subdirectory(gvo)

add_executable(testg src/main-opengl)
add_executable(testv src/main-vulkan)

target_link_libraries(testg gvo)
target_link_libraries(testv gvo)

message("${OPENGL_INCLUDE_DIRS}")