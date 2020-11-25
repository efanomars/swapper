# Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   libstmm-swapper/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

project(stmm-swapper CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions("c++14")

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-swapper")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/charremoverevent.h"
        "${STMMI_HEADERS_DIR}/colorremoverevent.h"
        "${STMMI_HEADERS_DIR}/destroyerevent.h"
        "${STMMI_HEADERS_DIR}/gravityblock.h"
        "${STMMI_HEADERS_DIR}/gravityevent.h"
        "${STMMI_HEADERS_DIR}/stmm-swapper-config.h"
        "${STMMI_HEADERS_DIR}/swapperevent.h"
        "${STMMI_HEADERS_DIR}/tileremoverevent.h"
        "${STMMI_HEADERS_DIR}/topjunkevent.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/adjremover.h"
        "${STMMI_SOURCES_DIR}/adjremover.cc"
        "${STMMI_SOURCES_DIR}/charremoverevent.cc"
        "${STMMI_SOURCES_DIR}/colorremoverevent.cc"
        "${STMMI_SOURCES_DIR}/destroyerevent.cc"
        "${STMMI_SOURCES_DIR}/gravityblock.cc"
        "${STMMI_SOURCES_DIR}/gravityevent.cc"
        "${STMMI_SOURCES_DIR}/swapperevent.cc"
        "${STMMI_SOURCES_DIR}/tileremoverevent.cc"
        "${STMMI_SOURCES_DIR}/topjunkevent.cc"
        )

# Define library
add_library(stmm-swapper ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-swapper-config.cc")

include("stmm-swapper-defs.cmake")

target_include_directories(stmm-swapper        PUBLIC ${STMMSWAPPER_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-swapper SYSTEM PUBLIC ${STMMSWAPPER_EXTRA_INCLUDE_SDIRS})
target_include_directories(stmm-swapper BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-swapper>  # <prefix>/include/stmm-swapper
)

target_link_libraries(stmm-swapper ${STMMSWAPPER_EXTRA_LIBRARIES})

set_target_properties(stmm-swapper PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-swapper PROPERTIES
        VERSION ${STMM_SWAPPER_VERSION}
        SOVERSION ${STMM_SWAPPER_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-swapper-${STMM_SWAPPER_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-swapper)

include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")
# Set version for stmm-swapper-config.cc.in
set(STMMI_PKG_VERSION "${STMM_SWAPPER_VERSION}")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-swapper-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-swapper-config.cc" @ONLY)

include(FindPkgConfig)
if (PKG_CONFIG_FOUND)
    # Set install dir for pkg-config file
    set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    # Create pkg-config file for the library
    configure_file("${PROJECT_SOURCE_DIR}/stmm-swapper.pc.in"
                   "${PROJECT_BINARY_DIR}/stmm-swapper.pc" @ONLY)
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-swapper was configured with the following options:")
message(STATUS " STMMI_HEADERS:                   ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                   ${STMMI_SOURCES}")
message(STATUS " STMMSWAPPER_EXTRA_INCLUDE_DIRS:  ${STMMSWAPPER_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMSWAPPER_EXTRA_INCLUDE_SDIRS: ${STMMSWAPPER_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMSWAPPER_EXTRA_LIBRARIES:     ${STMMSWAPPER_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:           ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:      ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                 ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                  ${CMAKE_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                      ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                   ${BUILD_TESTING}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-swapper doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-swapper LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-swapper")

if (PKG_CONFIG_FOUND)
    install(FILES  "${PROJECT_BINARY_DIR}/stmm-swapper.pc"   DESTINATION "lib/pkgconfig")
endif()
