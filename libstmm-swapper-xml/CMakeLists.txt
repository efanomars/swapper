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

# File:   libstmm-swapper-xml/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

project(stmm-swapper-xml CXX)

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
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-swapper-xml")
# Header files
set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/stmm-swapper-xml-config.h"
        "${STMMI_HEADERS_DIR}/xmlcolorremoverevent.h"
        "${STMMI_HEADERS_DIR}/xmldestroyerevent.h"
        "${STMMI_HEADERS_DIR}/xmlgravityevent.h"
        "${STMMI_HEADERS_DIR}/xmlswapperevent.h"
        "${STMMI_HEADERS_DIR}/xmltileremoverevent.h"
        "${STMMI_HEADERS_DIR}/xmltopjunkevent.h"
        )
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/xmlcolorremoverevent.cc"
        "${STMMI_SOURCES_DIR}/xmldestroyerevent.cc"
        "${STMMI_SOURCES_DIR}/xmlgravityevent.cc"
        "${STMMI_SOURCES_DIR}/xmlswapperevent.cc"
        "${STMMI_SOURCES_DIR}/xmltileremoverevent.cc"
        "${STMMI_SOURCES_DIR}/xmltopjunkevent.cc"
        )

# Data dir
set(STMMI_DATA_DIR  "${PROJECT_SOURCE_DIR}/data")
# Data files
set(STMMI_DATA_FILES
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/theme.xml"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/thumbnail.png"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/darkening.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperleftplayer0.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperleftplayer1.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperleftplayer2.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperleftplayer3.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperleftplayer4.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperrightplayer0.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperrightplayer1.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperrightplayer2.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperrightplayer3.svg"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/images/swapperrightplayer4.svg"
        #
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/sounds/dangerend.mp3"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/sounds/dangerstart.mp3"
        "${STMMI_DATA_DIR}/themes/swapper/simple_s.thm/sounds/junk.mp3"
        )

# Define library
add_library(stmm-swapper-xml ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-swapper-xml-config.cc")

include("stmm-swapper-xml-defs.cmake")

target_include_directories(stmm-swapper-xml        PUBLIC ${STMMSWAPPERXML_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-swapper-xml SYSTEM PUBLIC ${STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS})
target_include_directories(stmm-swapper-xml BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-swapper-xml>  # <prefix>/include/stmm-swapper-xml
)

target_link_libraries(stmm-swapper-xml ${STMMSWAPPERXML_EXTRA_LIBRARIES})

set_target_properties(stmm-swapper-xml PROPERTIES ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-swapper-xml PROPERTIES
        VERSION ${STMM_SWAPPER_XML_VERSION}
        SOVERSION ${STMM_SWAPPER_XML_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-swapper-xml-${STMM_SWAPPER_XML_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-swapper-xml)

# Set version for stmm-swapper-xml-config.cc.in
set(STMMI_PKG_VERSION "${STMM_SWAPPER_XML_VERSION}")
include(GNUInstallDirs)
set(STMMI_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")

# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-swapper-xml-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-swapper-xml-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-swapper-xml.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-swapper-xml.pc" @ONLY)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-swapper-xml was configured with the following options:")
message(STATUS " STMMI_HEADERS:                      ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                      ${STMMI_SOURCES}")
message(STATUS " STMMSWAPPERXML_EXTRA_INCLUDE_DIRS:  ${STMMSWAPPERXML_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS: ${STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS}")
message(STATUS " STMMSWAPPERXML_EXTRA_LIBRARIES:     ${STMMSWAPPERXML_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:                   ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:              ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:         ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                    ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                     ${STMMI_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                         ${BUILD_DOCS}")
message(STATUS " ENV{STMM_DEBIAN_PACKAGING}:         $ENV{STMM_DEBIAN_PACKAGING}")
message(STATUS " BUILD_TESTING:                      ${BUILD_TESTING}")
message(STATUS " STMMI_PKG_DATA_DIR:                 ${STMMI_PKG_DATA_DIR}")
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-swapper-xml doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-swapper-xml LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS} DESTINATION "include/stmm-swapper-xml")

install(FILES "${PROJECT_BINARY_DIR}/stmm-swapper-xml.pc" DESTINATION "lib/pkgconfig")

foreach (STMMI_DATA_CUR_FILE ${STMMI_DATA_FILES})
    file(RELATIVE_PATH  STMMI_DATA_CUR_REL_FILE  "${STMMI_DATA_DIR}"  "${STMMI_DATA_CUR_FILE}")
    get_filename_component(STMMI_DATA_CUR_DIR "${STMMI_PKG_DATA_DIR}/${STMMI_DATA_CUR_REL_FILE}" DIRECTORY)
    #message(STATUS "STMMI_DATA_CUR_FILE         ${STMMI_DATA_CUR_FILE}")
    #message(STATUS "STMMI_DATA_CUR_REL_FILE     ${STMMI_DATA_CUR_REL_FILE}")
    #message(STATUS "STMMI_DATA_CUR_DIR          ${STMMI_DATA_CUR_DIR}")
    install(FILES ${STMMI_DATA_CUR_FILE}  DESTINATION   "${STMMI_DATA_CUR_DIR}")
endforeach()
