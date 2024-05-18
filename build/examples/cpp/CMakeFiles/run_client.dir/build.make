# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build

# Include any dependencies generated for this target.
include examples/cpp/CMakeFiles/run_client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/cpp/CMakeFiles/run_client.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/cpp/CMakeFiles/run_client.dir/progress.make

# Include the compile flags for this target's objects.
include examples/cpp/CMakeFiles/run_client.dir/flags.make

examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o: examples/cpp/CMakeFiles/run_client.dir/flags.make
examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o: /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/examples/cpp/dg_core_client.cpp
examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o: examples/cpp/CMakeFiles/run_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o"
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o -MF CMakeFiles/run_client.dir/dg_core_client.cpp.o.d -o CMakeFiles/run_client.dir/dg_core_client.cpp.o -c /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/examples/cpp/dg_core_client.cpp

examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/run_client.dir/dg_core_client.cpp.i"
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/examples/cpp/dg_core_client.cpp > CMakeFiles/run_client.dir/dg_core_client.cpp.i

examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/run_client.dir/dg_core_client.cpp.s"
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/examples/cpp/dg_core_client.cpp -o CMakeFiles/run_client.dir/dg_core_client.cpp.s

# Object files for target run_client
run_client_OBJECTS = \
"CMakeFiles/run_client.dir/dg_core_client.cpp.o"

# External object files for target run_client
run_client_EXTERNAL_OBJECTS =

/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client: examples/cpp/CMakeFiles/run_client.dir/dg_core_client.cpp.o
/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client: examples/cpp/CMakeFiles/run_client.dir/build.make
/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client: /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/libaiclientlib.a
/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client: examples/cpp/CMakeFiles/run_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client"
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/run_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/cpp/CMakeFiles/run_client.dir/build: /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/bin/run_client
.PHONY : examples/cpp/CMakeFiles/run_client.dir/build

examples/cpp/CMakeFiles/run_client.dir/clean:
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp && $(CMAKE_COMMAND) -P CMakeFiles/run_client.dir/cmake_clean.cmake
.PHONY : examples/cpp/CMakeFiles/run_client.dir/clean

examples/cpp/CMakeFiles/run_client.dir/depend:
	cd /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/examples/cpp /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp /home/degirum/actions-runner/_work/Framework/Framework/AIModelRelease/build/examples/cpp/CMakeFiles/run_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/cpp/CMakeFiles/run_client.dir/depend
