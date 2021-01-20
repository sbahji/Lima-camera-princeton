###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2020
#  European Synchrotron Radiation Facility
#  CS40220 38043 Grenoble Cedex 9
#  FRANCE
#
#  Contact: lima@esrf.fr
#
#  This is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
cmake_minimum_required(VERSION 3.1)

project(princeton)

# Include additional modules that are used inconditionnaly
include(GNUInstallDirs)
include(GenerateExportHeader)

# If conda build, always set lib dir to 'lib'
if($ENV{CONDA_BUILD})
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Set lower / upper case project names
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

# Set targets export name (used by lima and dependencies)
set(TARGETS_EXPORT_NAME "${PROJECT_NAME_LOWER}-targets")
#set(namespace "${PROJECT_NAME}::")

# Check if project is being used directly or via add_subdirectory
set(CAMERA_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CAMERA_MASTER_PROJECT ON)
endif()

if (CAMERA_MASTER_PROJECT)
  find_package(Lima REQUIRED COMPONENTS devel tools)
endif()

# CMake additional macros
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${LIMA_CMAKE_INCLUDE_DIRS} ${CMAKE_MODULE_PATH})

include(LimaTools)

# Set version
include(project_version)

find_package(Picam REQUIRED)

if(UNIX)
	  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-needed")
endif()

#SOURCEs
file(GLOB_RECURSE PRINCETON_INCS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE PRINCETON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
# Library definition
add_library(princeton SHARED
  ${PRINCETON_SRC}
  ${PRINCETON_INCS})

# Generate export macros
generate_export_header(princeton)


#INCLUDEs
# Set version and output name
set_target_properties(princeton PROPERTIES
  OUTPUT_NAME "lima${PROJECT_NAME_LOWER}"
  VERSION "${PROJECT_VERSION}"
  SOVERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}")

# Add dependencies
#target_include_directories(princeton PUBLIC "include")
target_include_directories(princeton PUBLIC ${PICAM_INCLUDE_DIRS})
#target_include_directories(princeton PUBLIC  "/opt/PrincetonInstruments/picam/includes")
target_include_directories(princeton
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"           # For export header
    PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

target_link_libraries(princeton PUBLIC ${PICAM_LIBRARIES})
target_link_libraries(princeton PUBLIC limacore)


install(TARGETS princeton LIBRARY DESTINATION lib)

if(WIN32)
  target_compile_definitions(princeton
    PRIVATE princeton_EXPORTS
    PUBLIC NOMINMAX
    PUBLIC WIN32_LEAN_AND_MEAN)
  set_target_properties(princeton PROPERTIES PREFIX "lib" IMPORT_PREFIX "lib")
endif()

if(LIMA_ENABLE_PYTHON)
    limatools_run_sip_for_camera(princeton)
    install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/Princeton")
    if (LIMA_ENABLE_PYTANGO_SERVER)
      add_subdirectory(tango)
    endif()
endif()
