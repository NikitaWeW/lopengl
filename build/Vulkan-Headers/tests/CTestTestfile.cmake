# CMake generated Testfile for 
# Source directory: D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests
# Build directory: D:/projects/learning-opengl/build/Vulkan-Headers/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(integration.add_subdirectory "C:/cmake/bin/ctest.exe" "--build-and-test" "D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/integration" "D:/projects/learning-opengl/build/Vulkan-Headers/tests/add_subdirectory" "--build-generator" "MinGW Makefiles" "--build-options" "-DFIND_PACKAGE_TESTING=OFF" "-DVULKAN_HEADERS_ENABLE_MODULE=OFF")
set_tests_properties(integration.add_subdirectory PROPERTIES  _BACKTRACE_TRIPLES "D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;10;add_test;D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;0;")
add_test(integration.install "C:/cmake/bin/cmake.exe" "--install" "D:/projects/learning-opengl/build/Vulkan-Headers" "--prefix" "D:/projects/learning-opengl/build/Vulkan-Headers/tests/install" "--config" "")
set_tests_properties(integration.install PROPERTIES  _BACKTRACE_TRIPLES "D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;19;add_test;D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;0;")
add_test(integration.find_package "C:/cmake/bin/ctest.exe" "--build-and-test" "D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/integration" "D:/projects/learning-opengl/build/Vulkan-Headers/tests/find_package" "--build-generator" "MinGW Makefiles" "--build-options" "-DFIND_PACKAGE_TESTING=ON" "-DCMAKE_PREFIX_PATH=D:/projects/learning-opengl/build/Vulkan-Headers/tests/install")
set_tests_properties(integration.find_package PROPERTIES  DEPENDS "integration.install" _BACKTRACE_TRIPLES "D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;24;add_test;D:/projects/learning-opengl/gvo/dependencies/vulkan/Vulkan-Headers/tests/CMakeLists.txt;0;")
