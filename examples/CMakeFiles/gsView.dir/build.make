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
include examples/CMakeFiles/gsView.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/gsView.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/gsView.dir/flags.make

examples/CMakeFiles/gsView.dir/gsView.cpp.o: examples/CMakeFiles/gsView.dir/flags.make
examples/CMakeFiles/gsView.dir/gsView.cpp.o: /home/samwise/weinmueller/gismo/examples/gsView.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/gsView.dir/gsView.cpp.o"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gsView.dir/gsView.cpp.o -c /home/samwise/weinmueller/gismo/examples/gsView.cpp

examples/CMakeFiles/gsView.dir/gsView.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gsView.dir/gsView.cpp.i"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/samwise/weinmueller/gismo/examples/gsView.cpp > CMakeFiles/gsView.dir/gsView.cpp.i

examples/CMakeFiles/gsView.dir/gsView.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gsView.dir/gsView.cpp.s"
	cd /samwise/home/weinmueller/gismo/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/samwise/weinmueller/gismo/examples/gsView.cpp -o CMakeFiles/gsView.dir/gsView.cpp.s

examples/CMakeFiles/gsView.dir/gsView.cpp.o.requires:

.PHONY : examples/CMakeFiles/gsView.dir/gsView.cpp.o.requires

examples/CMakeFiles/gsView.dir/gsView.cpp.o.provides: examples/CMakeFiles/gsView.dir/gsView.cpp.o.requires
	$(MAKE) -f examples/CMakeFiles/gsView.dir/build.make examples/CMakeFiles/gsView.dir/gsView.cpp.o.provides.build
.PHONY : examples/CMakeFiles/gsView.dir/gsView.cpp.o.provides

examples/CMakeFiles/gsView.dir/gsView.cpp.o.provides.build: examples/CMakeFiles/gsView.dir/gsView.cpp.o


# Object files for target gsView
gsView_OBJECTS = \
"CMakeFiles/gsView.dir/gsView.cpp.o"

# External object files for target gsView
gsView_EXTERNAL_OBJECTS =

bin/gsView: examples/CMakeFiles/gsView.dir/gsView.cpp.o
bin/gsView: examples/CMakeFiles/gsView.dir/build.make
bin/gsView: lib/libgismo.so.0.8.5
bin/gsView: /usr/lib64/libz.so
bin/gsView: examples/CMakeFiles/gsView.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/gsView"
	cd /samwise/home/weinmueller/gismo/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gsView.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/gsView.dir/build: bin/gsView

.PHONY : examples/CMakeFiles/gsView.dir/build

examples/CMakeFiles/gsView.dir/requires: examples/CMakeFiles/gsView.dir/gsView.cpp.o.requires

.PHONY : examples/CMakeFiles/gsView.dir/requires

examples/CMakeFiles/gsView.dir/clean:
	cd /samwise/home/weinmueller/gismo/examples && $(CMAKE_COMMAND) -P CMakeFiles/gsView.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/gsView.dir/clean

examples/CMakeFiles/gsView.dir/depend:
	cd /samwise/home/weinmueller/gismo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/samwise/weinmueller/gismo /home/samwise/weinmueller/gismo/examples /samwise/home/weinmueller/gismo /samwise/home/weinmueller/gismo/examples /samwise/home/weinmueller/gismo/examples/CMakeFiles/gsView.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/gsView.dir/depend

