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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/hgfs/github/jacServer/jacServer/cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/github/jacServer/jacServer/cmake

# Include any dependencies generated for this target.
include CMakeFiles/jacClient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/jacClient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/jacClient.dir/flags.make

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o: CMakeFiles/jacClient.dir/flags.make
CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o: /mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/hgfs/github/jacServer/jacServer/cmake/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o"
	g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o -c /mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.i"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc > CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.i

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.s"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc -o CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.s

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.requires:
.PHONY : CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.requires

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.provides: CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.requires
	$(MAKE) -f CMakeFiles/jacClient.dir/build.make CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.provides.build
.PHONY : CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.provides

CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.provides.build: CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o

# Object files for target jacClient
jacClient_OBJECTS = \
"CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o"

# External object files for target jacClient
jacClient_EXTERNAL_OBJECTS =

bin/jacClient: CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o
bin/jacClient: CMakeFiles/jacClient.dir/build.make
bin/jacClient: /mnt/hgfs/github/jacServer/build/release-install/lib/libmuduo_net.a
bin/jacClient: /mnt/hgfs/github/jacServer/build/release-install/lib/libmuduo_base.a
bin/jacClient: CMakeFiles/jacClient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable bin/jacClient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/jacClient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/jacClient.dir/build: bin/jacClient
.PHONY : CMakeFiles/jacClient.dir/build

CMakeFiles/jacClient.dir/requires: CMakeFiles/jacClient.dir/mnt/hgfs/github/jacServer/jacServer/src/jacClient.cc.o.requires
.PHONY : CMakeFiles/jacClient.dir/requires

CMakeFiles/jacClient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/jacClient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/jacClient.dir/clean

CMakeFiles/jacClient.dir/depend:
	cd /mnt/hgfs/github/jacServer/jacServer/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/github/jacServer/jacServer/cmake /mnt/hgfs/github/jacServer/jacServer/cmake /mnt/hgfs/github/jacServer/jacServer/cmake /mnt/hgfs/github/jacServer/jacServer/cmake /mnt/hgfs/github/jacServer/jacServer/cmake/CMakeFiles/jacClient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/jacClient.dir/depend

