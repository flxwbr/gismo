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
include doc/snippets/CMakeFiles/bspline_greville.dir/depend.make

# Include the progress variables for this target.
include doc/snippets/CMakeFiles/bspline_greville.dir/progress.make

# Include the compile flags for this target's objects.
include doc/snippets/CMakeFiles/bspline_greville.dir/flags.make

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o: doc/snippets/CMakeFiles/bspline_greville.dir/flags.make
doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o: doc/snippets/bspline_greville.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o"
	cd /samwise/home/weinmueller/gismo/doc/snippets && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o -c /samwise/home/weinmueller/gismo/doc/snippets/bspline_greville.cpp

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bspline_greville.dir/bspline_greville.cpp.i"
	cd /samwise/home/weinmueller/gismo/doc/snippets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /samwise/home/weinmueller/gismo/doc/snippets/bspline_greville.cpp > CMakeFiles/bspline_greville.dir/bspline_greville.cpp.i

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bspline_greville.dir/bspline_greville.cpp.s"
	cd /samwise/home/weinmueller/gismo/doc/snippets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /samwise/home/weinmueller/gismo/doc/snippets/bspline_greville.cpp -o CMakeFiles/bspline_greville.dir/bspline_greville.cpp.s

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.requires:

.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.requires

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.provides: doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.requires
	$(MAKE) -f doc/snippets/CMakeFiles/bspline_greville.dir/build.make doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.provides.build
.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.provides

doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.provides.build: doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o


# Object files for target bspline_greville
bspline_greville_OBJECTS = \
"CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o"

# External object files for target bspline_greville
bspline_greville_EXTERNAL_OBJECTS =

bin/bspline_greville: doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o
bin/bspline_greville: doc/snippets/CMakeFiles/bspline_greville.dir/build.make
bin/bspline_greville: lib/libgismo.so.0.8.5
bin/bspline_greville: /usr/lib64/libz.so
bin/bspline_greville: doc/snippets/CMakeFiles/bspline_greville.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/samwise/home/weinmueller/gismo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/bspline_greville"
	cd /samwise/home/weinmueller/gismo/doc/snippets && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bspline_greville.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
doc/snippets/CMakeFiles/bspline_greville.dir/build: bin/bspline_greville

.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/build

doc/snippets/CMakeFiles/bspline_greville.dir/requires: doc/snippets/CMakeFiles/bspline_greville.dir/bspline_greville.cpp.o.requires

.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/requires

doc/snippets/CMakeFiles/bspline_greville.dir/clean:
	cd /samwise/home/weinmueller/gismo/doc/snippets && $(CMAKE_COMMAND) -P CMakeFiles/bspline_greville.dir/cmake_clean.cmake
.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/clean

doc/snippets/CMakeFiles/bspline_greville.dir/depend:
	cd /samwise/home/weinmueller/gismo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/samwise/weinmueller/gismo /home/samwise/weinmueller/gismo/doc/snippets /samwise/home/weinmueller/gismo /samwise/home/weinmueller/gismo/doc/snippets /samwise/home/weinmueller/gismo/doc/snippets/CMakeFiles/bspline_greville.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : doc/snippets/CMakeFiles/bspline_greville.dir/depend

