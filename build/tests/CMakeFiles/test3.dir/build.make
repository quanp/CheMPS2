# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ward/Documents/phd/CheMPS2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ward/Documents/phd/CheMPS2/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/test3.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test3.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test3.dir/flags.make

tests/CMakeFiles/test3.dir/test3.cpp.o: tests/CMakeFiles/test3.dir/flags.make
tests/CMakeFiles/test3.dir/test3.cpp.o: ../tests/test3.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ward/Documents/phd/CheMPS2/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tests/CMakeFiles/test3.dir/test3.cpp.o"
	cd /home/ward/Documents/phd/CheMPS2/build/tests && /usr/lib64/ccache/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/test3.dir/test3.cpp.o -c /home/ward/Documents/phd/CheMPS2/tests/test3.cpp

tests/CMakeFiles/test3.dir/test3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test3.dir/test3.cpp.i"
	cd /home/ward/Documents/phd/CheMPS2/build/tests && /usr/lib64/ccache/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ward/Documents/phd/CheMPS2/tests/test3.cpp > CMakeFiles/test3.dir/test3.cpp.i

tests/CMakeFiles/test3.dir/test3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test3.dir/test3.cpp.s"
	cd /home/ward/Documents/phd/CheMPS2/build/tests && /usr/lib64/ccache/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ward/Documents/phd/CheMPS2/tests/test3.cpp -o CMakeFiles/test3.dir/test3.cpp.s

tests/CMakeFiles/test3.dir/test3.cpp.o.requires:
.PHONY : tests/CMakeFiles/test3.dir/test3.cpp.o.requires

tests/CMakeFiles/test3.dir/test3.cpp.o.provides: tests/CMakeFiles/test3.dir/test3.cpp.o.requires
	$(MAKE) -f tests/CMakeFiles/test3.dir/build.make tests/CMakeFiles/test3.dir/test3.cpp.o.provides.build
.PHONY : tests/CMakeFiles/test3.dir/test3.cpp.o.provides

tests/CMakeFiles/test3.dir/test3.cpp.o.provides.build: tests/CMakeFiles/test3.dir/test3.cpp.o

# Object files for target test3
test3_OBJECTS = \
"CMakeFiles/test3.dir/test3.cpp.o"

# External object files for target test3
test3_EXTERNAL_OBJECTS =

tests/test3: tests/CMakeFiles/test3.dir/test3.cpp.o
tests/test3: tests/CMakeFiles/test3.dir/build.make
tests/test3: CheMPS2/libCheMPS2.so
tests/test3: /usr/lib64/liblapack.so
tests/test3: /usr/lib64/libblas.so
tests/test3: /usr/lib64/libhdf5.so
tests/test3: /usr/lib64/libz.so
tests/test3: /usr/lib64/libm.so
tests/test3: tests/CMakeFiles/test3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable test3"
	cd /home/ward/Documents/phd/CheMPS2/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test3.dir/build: tests/test3
.PHONY : tests/CMakeFiles/test3.dir/build

tests/CMakeFiles/test3.dir/requires: tests/CMakeFiles/test3.dir/test3.cpp.o.requires
.PHONY : tests/CMakeFiles/test3.dir/requires

tests/CMakeFiles/test3.dir/clean:
	cd /home/ward/Documents/phd/CheMPS2/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/test3.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test3.dir/clean

tests/CMakeFiles/test3.dir/depend:
	cd /home/ward/Documents/phd/CheMPS2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ward/Documents/phd/CheMPS2 /home/ward/Documents/phd/CheMPS2/tests /home/ward/Documents/phd/CheMPS2/build /home/ward/Documents/phd/CheMPS2/build/tests /home/ward/Documents/phd/CheMPS2/build/tests/CMakeFiles/test3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/test3.dir/depend

