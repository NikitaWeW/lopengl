cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

function(create_resources dir output)
    # thanks to https://stackoverflow.com/a/27206982/23847181
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename "${bin}")
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename "${filename}")
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata "${filedata}")
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename})\n;\n")
    endforeach()
endfunction()

project(ogl_setup C CXX)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE DEPENDENCIES_SOURCES "dependencies/compile/*")

if(WIN32)
    file(GLOB_RECURSE LIBRARIES "dependencies/lib/windows/*")
elseif(UNIX AND NOT APPLE)
    file(GLOB_RECURSE LIBRARIES "dependencies/lib/linux/*")
else()
    message(WARNING "Not windows nor linux detected! You need to set LIBRARIES variable containing manualy.")
endif() 

# TODO: embed resources
create_resources("res",     "${CMAKE_BINARY_DIR}/res.c")
create_resources("shaders", "${CMAKE_BINARY_DIR}/shaders.c")

add_executable(main ${SOURCES} ${DEPENDENCIES_SOURCES})

target_link_libraries(main PRIVATE ${LIBRARIES})
target_include_directories(main PRIVATE dependencies/include src)
