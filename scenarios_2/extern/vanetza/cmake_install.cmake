# Install script for directory: /home/ilya-cavise/artery/extern/vanetza

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza" TYPE FILE FILES
    "/home/ilya-cavise/artery/scenarios/extern/vanetza/cmake-config/VanetzaConfig.cmake"
    "/home/ilya-cavise/artery/scenarios/extern/vanetza/cmake-config/VanetzaConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza" TYPE DIRECTORY FILES "/home/ilya-cavise/artery/extern/vanetza/cmake/" FILES_MATCHING REGEX "/MacroFindDependencyComponents\\.cmake$" REGEX "/Find[^/]*\\.cmake$" REGEX "/Compat[^/]*Targets\\.cmake$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza/VanetzaTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza/VanetzaTargets.cmake"
         "/home/ilya-cavise/artery/scenarios/extern/vanetza/CMakeFiles/Export/d8a6cea77e55297ab7ae686d6d16bf0a/VanetzaTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza/VanetzaTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza/VanetzaTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza" TYPE FILE FILES "/home/ilya-cavise/artery/scenarios/extern/vanetza/CMakeFiles/Export/d8a6cea77e55297ab7ae686d6d16bf0a/VanetzaTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vanetza" TYPE FILE FILES "/home/ilya-cavise/artery/scenarios/extern/vanetza/CMakeFiles/Export/d8a6cea77e55297ab7ae686d6d16bf0a/VanetzaTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/ilya-cavise/artery/extern/vanetza/vanetza" FILES_MATCHING REGEX "/[^/]*\\.hpp$" REGEX "/\\/tests\\/[^/]*\\.hpp$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/access/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/asn1/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/btp/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/common/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/dcc/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/facilities/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/geonet/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/gnss/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/net/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/security/cmake_install.cmake")
  include("/home/ilya-cavise/artery/scenarios/extern/vanetza/vanetza/units/cmake_install.cmake")

endif()

