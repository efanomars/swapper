# File: swapper/CMakeLists.txt

#  Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, see <http://www.gnu.org/licenses/>

# if(POLICY CMP0065)
#     cmake_policy(SET CMP0065 NEW)
# endif()

cmake_minimum_required(VERSION 3.0)

option(STMM_INSTALL_MAN_PAGE "Install man page in debian.orig/" ON)
option(STMM_INSTALL_LAUNCHER "Install launcher in share/applications/ (implies STMM_INSTALL_ICONS=ON)" ON)
option(STMM_INSTALL_ICONS "Install icons in share/icons/hicolor/(size)/apps/" ON)

project(swapper CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineCommonCompileOptions("c++14")

# Source files (and headers only used for building)
set(STMMI_SWA_SOURCES
        "${PROJECT_SOURCE_DIR}/src/binconfig.h"
        "${PROJECT_SOURCE_DIR}/src/main.cc"
        "${PROJECT_SOURCE_DIR}/src/setup.h"
        "${PROJECT_SOURCE_DIR}/src/setup.cc"
        "${PROJECT_SOURCE_DIR}/src/setupstdconfig.h"
        "${PROJECT_SOURCE_DIR}/src/setupstdconfig.cc"
        "${PROJECT_SOURCE_DIR}/src/setupxmlgameloader.h"
        "${PROJECT_SOURCE_DIR}/src/setupxmlgameloader.cc"
        "${PROJECT_SOURCE_DIR}/src/setupxmlthemeloader.h"
        "${PROJECT_SOURCE_DIR}/src/setupxmlthemeloader.cc"
        )
set(STMMI_SWA_DATA_DIR  "${PROJECT_SOURCE_DIR}/data")
set(STMMI_SWA_DATA_GAMES_DIR  "${STMMI_SWA_DATA_DIR}/games/swapper")
set(STMMI_SWA_DATA_GAMES_FILES
        "${STMMI_SWA_DATA_GAMES_DIR}/blob.xml"
        "${STMMI_SWA_DATA_GAMES_DIR}/classic.xml"
        "${STMMI_SWA_DATA_GAMES_DIR}/classic2H.xml"
        "${STMMI_SWA_DATA_GAMES_DIR}/stony.xml"
        "${STMMI_SWA_DATA_GAMES_DIR}/classicVs.xml"
        "${STMMI_SWA_DATA_GAMES_DIR}/stonyVs.xml"
        )
set(STMMI_SWA_DATA_GAMES_IMG_FILES
        "${STMMI_SWA_DATA_GAMES_DIR}/blob.png"
        "${STMMI_SWA_DATA_GAMES_DIR}/classic.png"
        "${STMMI_SWA_DATA_GAMES_DIR}/classic2H.png"
        "${STMMI_SWA_DATA_GAMES_DIR}/stony.png"
        "${STMMI_SWA_DATA_GAMES_DIR}/classicVs.png"
        "${STMMI_SWA_DATA_GAMES_DIR}/stonyVs.png"
        )
set(STMMI_SWA_DATA_OTHER_FILES
        "${STMMI_SWA_DATA_DIR}/games/swapper.png"
        )

add_executable(swapper ${STMMI_SWA_SOURCES} "${PROJECT_BINARY_DIR}/binconfig.cc")

include("swapper-defs.cmake")

target_include_directories(swapper        PUBLIC ${SWAPPER_EXTRA_INCLUDE_DIRS})
target_include_directories(swapper SYSTEM PUBLIC ${SWAPPER_EXTRA_INCLUDE_SDIRS})
# This allows binconfig.cc to find the binconfig.h include
target_include_directories(swapper BEFORE PUBLIC "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(swapper ${SWAPPER_EXTRA_LIBRARIES})

DefineTargetPublicCompileOptions(swapper)

# Set version for binconfig.cc.in
set(STMMI_PKG_VERSION "${SWAPPER_VERSION}")

include(GNUInstallDirs)
set(STMMI_SWA_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-games")
set(STMMI_SWA_PKG_REL_DATA_DIR  "${CMAKE_INSTALL_DATADIR}/stmm-games")
 
# Create binconfig file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/binconfig.cc.in"
               "${PROJECT_BINARY_DIR}/binconfig.cc" @ONLY)
if (STMM_INSTALL_MAN_PAGE)
    configure_file("${PROJECT_SOURCE_DIR}/../scripts/priv/debian.orig/swapper.1"
                   "${PROJECT_BINARY_DIR}/swapper.1" @ONLY)
    CreateManGz(swapper "swapper.1")
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "swapper was configured with the following options:")
message(STATUS " STMMI_SWA_SOURCES:           ${STMMI_SWA_SOURCES}")
message(STATUS " SWAPPER_EXTRA_INCLUDE_DIRS:  ${SWAPPER_EXTRA_INCLUDE_DIRS}")
message(STATUS " SWAPPER_EXTRA_INCLUDE_SDIRS: ${SWAPPER_EXTRA_INCLUDE_SDIRS}")
message(STATUS " SWAPPER_EXTRA_LIBRARIES:     ${SWAPPER_EXTRA_LIBRARIES}")
message(STATUS " STMMI_SWA_DATA_FILES         ${STMMI_SWA_DATA_FILES}")
message(STATUS " CMAKE_BUILD_TYPE:            ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:       ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:  ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " STMMI_SWA_PKG_REL_DATA_DIR   ${STMMI_SWA_PKG_REL_DATA_DIR}")
message(STATUS " STMMI_SWA_DATA_DIR           ${STMMI_SWA_DATA_DIR}")
message(STATUS " STMMI_SWA_DATA_GAMES_DIR     ${STMMI_SWA_DATA_GAMES_DIR}")
message(STATUS " install prefix:              ${CMAKE_INSTALL_PREFIX}")
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS swapper RUNTIME DESTINATION "bin")

install(FILES ${STMMI_SWA_DATA_GAMES_FILES}     DESTINATION "${STMMI_SWA_PKG_REL_DATA_DIR}/games/swapper")
install(FILES ${STMMI_SWA_DATA_GAMES_IMG_FILES} DESTINATION "${STMMI_SWA_PKG_REL_DATA_DIR}/games/swapper")
install(FILES ${STMMI_SWA_DATA_OTHER_FILES}     DESTINATION "${STMMI_SWA_PKG_REL_DATA_DIR}/games")

if (STMM_INSTALL_LAUNCHER)
    install(FILES           "${STMMI_SWA_DATA_DIR}/applications/com.efanomars.swapper.desktop"
            DESTINATION                "${CMAKE_INSTALL_DATADIR}/applications/")
endif()
if (STMM_INSTALL_ICONS OR STMM_INSTALL_LAUNCHER)
    install(FILES             "${STMMI_SWA_DATA_DIR}/icons/hicolor/24x24/apps/swapper.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps/")
    install(FILES             "${STMMI_SWA_DATA_DIR}/icons/hicolor/32x32/apps/swapper.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps/")
    install(FILES             "${STMMI_SWA_DATA_DIR}/icons/hicolor/48x48/apps/swapper.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/")
    install(FILES          "${STMMI_SWA_DATA_DIR}/icons/hicolor/scalable/apps/swapper.svg"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/")
endif()

if (STMM_INSTALL_MAN_PAGE)
    install(FILES                                      "${PROJECT_BINARY_DIR}/swapper.1.gz"
            DESTINATION                    "${CMAKE_INSTALL_DATADIR}/man/man1/")
endif()

