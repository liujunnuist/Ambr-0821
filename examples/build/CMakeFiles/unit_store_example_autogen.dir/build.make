# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/liujun/Ambr/examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liujun/Ambr/examples/build

# Utility rule file for unit_store_example_autogen.

# Include the progress variables for this target.
include CMakeFiles/unit_store_example_autogen.dir/progress.make

CMakeFiles/unit_store_example_autogen:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/liujun/Ambr/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC, UIC and RCC for target unit_store_example"
	/usr/bin/cmake -E cmake_autogen /home/liujun/Ambr/examples/build/CMakeFiles/unit_store_example_autogen.dir ""

unit_store_example_autogen: CMakeFiles/unit_store_example_autogen
unit_store_example_autogen: CMakeFiles/unit_store_example_autogen.dir/build.make

.PHONY : unit_store_example_autogen

# Rule to build all files generated by this target.
CMakeFiles/unit_store_example_autogen.dir/build: unit_store_example_autogen

.PHONY : CMakeFiles/unit_store_example_autogen.dir/build

CMakeFiles/unit_store_example_autogen.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/unit_store_example_autogen.dir/cmake_clean.cmake
.PHONY : CMakeFiles/unit_store_example_autogen.dir/clean

CMakeFiles/unit_store_example_autogen.dir/depend:
	cd /home/liujun/Ambr/examples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liujun/Ambr/examples /home/liujun/Ambr/examples /home/liujun/Ambr/examples/build /home/liujun/Ambr/examples/build /home/liujun/Ambr/examples/build/CMakeFiles/unit_store_example_autogen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/unit_store_example_autogen.dir/depend

