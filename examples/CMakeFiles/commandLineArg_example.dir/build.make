# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/samwise/weinmueller/gismo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /samwise/home/weinmueller/gismo

# Include any dependencies generated for this target.
include examples/CMakeFiles/commandLineArg_example.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/commandLineArg_example.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/commandLineArg_example.dir/flags.make

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o: examples/CMakeFiles/commandLineArg_example.dir/flags.make
examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o: /home/samwise/weinmueller/gismo/examples/commandLineArg_example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o -c /home/samwise/weinmueller/gismo/examples/commandLineArg_example.cpp

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.i"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/samwise/weinmueller/gismo/examples/commandLineArg_example.cpp > CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.i

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.s"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/samwise/weinmueller/gismo/examples/commandLineArg_example.cpp -o CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.s

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.requires:

.PHONY : examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.requires

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.provides: examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/commandLineArg_example.dir/build.make examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.provides.build
.PHONY : examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.provides

examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.provides.build: examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o


# Object files for target commandLineArg_example
commandLineArg_example_OBJECTS = \
"CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o"

# External object files for target commandLineArg_example
commandLineArg_example_EXTERNAL_OBJECTS =

bin/commandLineArg_example: examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o
bin/commandLineArg_example: examples/CMakeFiles/commandLineArg_example.dir/build.make
bin/commandLineArg_example: lib/libgismo.so.0.8.5
bin/commandLineArg_example: /usr/lib64/libz.so
bin/commandLineArg_example: examples/CMakeFiles/commandLineArg_example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/commandLineArg_example"
	cd /samwise/home/weinmueller/gismo/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/commandLineArg_example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/commandLineArg_example.dir/build: bin/commandLineArg_example

.PHONY : examples/CMakeFiles/commandLineArg_example.dir/build

examples/CMakeFiles/commandLineArg_example.dir/requires: examples/CMakeFiles/commandLineArg_example.dir/commandLineArg_example.cpp.o.requires

.PHONY : examples/CMakeFiles/commandLineArg_example.dir/requires

examples/CMakeFiles/commandLineArg_example.dir/clean:
	cd /samwise/home/weinmueller/gismo/examples && $(CMAKE_COMMAND) -P CMakeFiles/commandLineArg_example.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/commandLineArg_example.dir/clean

examples/CMakeFiles/commandLineArg_example.dir/depend:
	cd /samwise/home/weinmueller/gismo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/samwise/weinmueller/gismo /home/samwise/weinmueller/gismo/examples /samwise/home/weinmueller/gismo /samwise/home/weinmueller/gismo/examples /samwise/home/weinmueller/gismo/examples/CMakeFiles/commandLineArg_example.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/commandLineArg_example.dir/depend

