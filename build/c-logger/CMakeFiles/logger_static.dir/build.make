# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\cmake\bin\cmake.exe

# The command to remove a file.
RM = C:\cmake\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\projects\learning-opengl

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\projects\learning-opengl\build

# Include any dependencies generated for this target.
include c-logger/CMakeFiles/logger_static.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include c-logger/CMakeFiles/logger_static.dir/compiler_depend.make

# Include the progress variables for this target.
include c-logger/CMakeFiles/logger_static.dir/progress.make

# Include the compile flags for this target's objects.
include c-logger/CMakeFiles/logger_static.dir/flags.make

c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj: c-logger/CMakeFiles/logger_static.dir/flags.make
c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj: c-logger/CMakeFiles/logger_static.dir/includes_C.rsp
c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj: D:/projects/learning-opengl/gvo/dependencies/c-logger/src/logger.c
c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj: c-logger/CMakeFiles/logger_static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=D:\projects\learning-opengl\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj -MF CMakeFiles\logger_static.dir\src\logger.c.obj.d -o CMakeFiles\logger_static.dir\src\logger.c.obj -c D:\projects\learning-opengl\gvo\dependencies\c-logger\src\logger.c

c-logger/CMakeFiles/logger_static.dir/src/logger.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/logger_static.dir/src/logger.c.i"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\projects\learning-opengl\gvo\dependencies\c-logger\src\logger.c > CMakeFiles\logger_static.dir\src\logger.c.i

c-logger/CMakeFiles/logger_static.dir/src/logger.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/logger_static.dir/src/logger.c.s"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S D:\projects\learning-opengl\gvo\dependencies\c-logger\src\logger.c -o CMakeFiles\logger_static.dir\src\logger.c.s

c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj: c-logger/CMakeFiles/logger_static.dir/flags.make
c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj: c-logger/CMakeFiles/logger_static.dir/includes_C.rsp
c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj: D:/projects/learning-opengl/gvo/dependencies/c-logger/src/loggerconf.c
c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj: c-logger/CMakeFiles/logger_static.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=D:\projects\learning-opengl\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj -MF CMakeFiles\logger_static.dir\src\loggerconf.c.obj.d -o CMakeFiles\logger_static.dir\src\loggerconf.c.obj -c D:\projects\learning-opengl\gvo\dependencies\c-logger\src\loggerconf.c

c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/logger_static.dir/src/loggerconf.c.i"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\projects\learning-opengl\gvo\dependencies\c-logger\src\loggerconf.c > CMakeFiles\logger_static.dir\src\loggerconf.c.i

c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/logger_static.dir/src/loggerconf.c.s"
	cd /d D:\projects\learning-opengl\build\c-logger && C:\msys64\mingw64\bin\cc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S D:\projects\learning-opengl\gvo\dependencies\c-logger\src\loggerconf.c -o CMakeFiles\logger_static.dir\src\loggerconf.c.s

# Object files for target logger_static
logger_static_OBJECTS = \
"CMakeFiles/logger_static.dir/src/logger.c.obj" \
"CMakeFiles/logger_static.dir/src/loggerconf.c.obj"

# External object files for target logger_static
logger_static_EXTERNAL_OBJECTS =

c-logger/liblogger.a: c-logger/CMakeFiles/logger_static.dir/src/logger.c.obj
c-logger/liblogger.a: c-logger/CMakeFiles/logger_static.dir/src/loggerconf.c.obj
c-logger/liblogger.a: c-logger/CMakeFiles/logger_static.dir/build.make
c-logger/liblogger.a: c-logger/CMakeFiles/logger_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=D:\projects\learning-opengl\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C static library liblogger.a"
	cd /d D:\projects\learning-opengl\build\c-logger && $(CMAKE_COMMAND) -P CMakeFiles\logger_static.dir\cmake_clean_target.cmake
	cd /d D:\projects\learning-opengl\build\c-logger && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\logger_static.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
c-logger/CMakeFiles/logger_static.dir/build: c-logger/liblogger.a
.PHONY : c-logger/CMakeFiles/logger_static.dir/build

c-logger/CMakeFiles/logger_static.dir/clean:
	cd /d D:\projects\learning-opengl\build\c-logger && $(CMAKE_COMMAND) -P CMakeFiles\logger_static.dir\cmake_clean.cmake
.PHONY : c-logger/CMakeFiles/logger_static.dir/clean

c-logger/CMakeFiles/logger_static.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\projects\learning-opengl D:\projects\learning-opengl\gvo\dependencies\c-logger D:\projects\learning-opengl\build D:\projects\learning-opengl\build\c-logger D:\projects\learning-opengl\build\c-logger\CMakeFiles\logger_static.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : c-logger/CMakeFiles/logger_static.dir/depend

