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
CMAKE_SOURCE_DIR = /root/github/jacsys/cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/github/jacsys/cmake

# Include any dependencies generated for this target.
include CMakeFiles/jacServer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/jacServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/jacServer.dir/flags.make

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o: CMakeFiles/jacServer.dir/flags.make
CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o: /root/github/jacsys/src/jacServer.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /root/github/jacsys/cmake/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o"
	g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o -c /root/github/jacsys/src/jacServer.cc

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.i"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /root/github/jacsys/src/jacServer.cc > CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.i

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.s"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /root/github/jacsys/src/jacServer.cc -o CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.s

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.requires:
.PHONY : CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.requires

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.provides: CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.requires
	$(MAKE) -f CMakeFiles/jacServer.dir/build.make CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.provides.build
.PHONY : CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.provides

CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.provides.build: CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o: CMakeFiles/jacServer.dir/flags.make
CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o: /root/github/jacsys/src/database_operator.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /root/github/jacsys/cmake/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o"
	g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o -c /root/github/jacsys/src/database_operator.cc

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.i"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /root/github/jacsys/src/database_operator.cc > CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.i

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.s"
	g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /root/github/jacsys/src/database_operator.cc -o CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.s

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.requires:
.PHONY : CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.requires

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.provides: CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.requires
	$(MAKE) -f CMakeFiles/jacServer.dir/build.make CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.provides.build
.PHONY : CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.provides

CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.provides.build: CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o

# Object files for target jacServer
jacServer_OBJECTS = \
"CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o" \
"CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o"

# External object files for target jacServer
jacServer_EXTERNAL_OBJECTS =

bin/jacServer: CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o
bin/jacServer: CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o
bin/jacServer: CMakeFiles/jacServer.dir/build.make
bin/jacServer: /root/build/release-install/lib/libmuduo_net.a
bin/jacServer: /root/build/release-install/lib/libmuduo_base.a
bin/jacServer: CMakeFiles/jacServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable bin/jacServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/jacServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/jacServer.dir/build: bin/jacServer
.PHONY : CMakeFiles/jacServer.dir/build

CMakeFiles/jacServer.dir/requires: CMakeFiles/jacServer.dir/root/github/jacsys/src/jacServer.cc.o.requires
CMakeFiles/jacServer.dir/requires: CMakeFiles/jacServer.dir/root/github/jacsys/src/database_operator.cc.o.requires
.PHONY : CMakeFiles/jacServer.dir/requires

CMakeFiles/jacServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/jacServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/jacServer.dir/clean

CMakeFiles/jacServer.dir/depend:
	cd /root/github/jacsys/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/github/jacsys/cmake /root/github/jacsys/cmake /root/github/jacsys/cmake /root/github/jacsys/cmake /root/github/jacsys/cmake/CMakeFiles/jacServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/jacServer.dir/depend

