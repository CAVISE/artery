# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_SOURCE_DIR = /home/ilya-cavise/artery

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ilya-cavise/artery/scenarios

# Utility rule file for run_mtits2017.

# Include any custom commands dependencies for this target.
include scenarios/CMakeFiles/run_mtits2017.dir/compiler_depend.make

# Include the progress variables for this target.
include scenarios/CMakeFiles/run_mtits2017.dir/progress.make

scenarios/CMakeFiles/run_mtits2017:
	cd /home/ilya-cavise/artery/scenarios/mt-its2017 && /home/ilya-cavise/omnetpp/bin/opp_run_dbg -n /home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases -l /home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so -l /home/ilya-cavise/artery/scenarios/scenarios/highway-police/libartery_police.so -l /home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so -l /home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so -l /home/ilya-cavise/artery/scenarios/extern/libINET.so -l /home/ilya-cavise/artery/scenarios/extern/libveins.so -l /home/ilya-cavise/artery/scenarios/src/traci/libtraci.so -l /home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so omnetpp.ini

run_mtits2017: scenarios/CMakeFiles/run_mtits2017
run_mtits2017: scenarios/CMakeFiles/run_mtits2017.dir/build.make
.PHONY : run_mtits2017

# Rule to build all files generated by this target.
scenarios/CMakeFiles/run_mtits2017.dir/build: run_mtits2017
.PHONY : scenarios/CMakeFiles/run_mtits2017.dir/build

scenarios/CMakeFiles/run_mtits2017.dir/clean:
	cd /home/ilya-cavise/artery/scenarios/scenarios && $(CMAKE_COMMAND) -P CMakeFiles/run_mtits2017.dir/cmake_clean.cmake
.PHONY : scenarios/CMakeFiles/run_mtits2017.dir/clean

scenarios/CMakeFiles/run_mtits2017.dir/depend:
	cd /home/ilya-cavise/artery/scenarios && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ilya-cavise/artery /home/ilya-cavise/artery/scenarios /home/ilya-cavise/artery/scenarios /home/ilya-cavise/artery/scenarios/scenarios /home/ilya-cavise/artery/scenarios/scenarios/CMakeFiles/run_mtits2017.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : scenarios/CMakeFiles/run_mtits2017.dir/depend

