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

# File:   stmm-swapper-xml-defs.cmake

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_SWAPPER_XML_MAJOR_VERSION 0)
set(STMM_SWAPPER_XML_MINOR_VERSION 30) # !-U-!
set(STMM_SWAPPER_XML_VERSION "${STMM_SWAPPER_XML_MAJOR_VERSION}.${STMM_SWAPPER_XML_MINOR_VERSION}.0")

# required stmm-swapper version
set(STMM_SWAPPER_XML_REQ_STMM_SWAPPER_MAJOR_VERSION 0)
set(STMM_SWAPPER_XML_REQ_STMM_SWAPPER_MINOR_VERSION 30) # !-U-!
set(STMM_SWAPPER_XML_REQ_STMM_SWAPPER_VERSION "${STMM_SWAPPER_XML_REQ_STMM_SWAPPER_MAJOR_VERSION}.${STMM_SWAPPER_XML_REQ_STMM_SWAPPER_MINOR_VERSION}")

# required stmm-games-xml-game version
set(STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_MAJOR_VERSION 0)
set(STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_MINOR_VERSION 30) # !-U-!
set(STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_VERSION "${STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_MAJOR_VERSION}.${STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMESXMLGAME   REQUIRED  stmm-games-xml-game>=${STMM_SWAPPER_XML_REQ_STMM_GAMES_XML_GAME_VERSION})
endif()

include("${PROJECT_SOURCE_DIR}/../libstmm-swapper/stmm-swapper-defs.cmake")

# include dirs
set(        STMMSWAPPERXML_EXTRA_INCLUDE_DIRS  "")
list(APPEND STMMSWAPPERXML_EXTRA_INCLUDE_DIRS  "${STMMSWAPPER_INCLUDE_DIRS}")
set(        STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS "")
list(APPEND STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS "${STMMSWAPPER_INCLUDE_SDIRS}")
list(APPEND STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS "${STMMGAMESXMLGAME_INCLUDE_DIRS}")

set(STMMI_TEMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../libstmm-swapper-xml/include")
set(STMMI_TEMP_HEADERS_DIR "${STMMI_TEMP_INCLUDE_DIR}/stmm-swapper-xml")

set(        STMMSWAPPERXML_INCLUDE_DIRS  "")
list(APPEND STMMSWAPPERXML_INCLUDE_DIRS  "${STMMI_TEMP_INCLUDE_DIR}")
list(APPEND STMMSWAPPERXML_INCLUDE_DIRS  "${STMMI_TEMP_HEADERS_DIR}")
list(APPEND STMMSWAPPERXML_INCLUDE_DIRS  "${STMMSWAPPERXML_EXTRA_INCLUDE_DIRS}")
set(        STMMSWAPPERXML_INCLUDE_SDIRS "")
list(APPEND STMMSWAPPERXML_INCLUDE_SDIRS "${STMMSWAPPERXML_EXTRA_INCLUDE_SDIRS}")

# libs
set(        STMMI_TEMP_EXTERNAL_LIBRARIES       "")
list(APPEND STMMI_TEMP_EXTERNAL_LIBRARIES       "${STMMGAMESXMLGAME_LIBRARIES}")

set(        STMMSWAPPERXML_EXTRA_LIBRARIES     "")
list(APPEND STMMSWAPPERXML_EXTRA_LIBRARIES     "${STMMSWAPPER_LIBRARIES}")
list(APPEND STMMSWAPPERXML_EXTRA_LIBRARIES     "${STMMI_TEMP_EXTERNAL_LIBRARIES}")

if (BUILD_SHARED_LIBS)
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-swapper-xml/build/libstmm-swapper-xml.so")
else()
    set(STMMI_LIB_FILE "${PROJECT_SOURCE_DIR}/../libstmm-swapper-xml/build/libstmm-swapper-xml.a")
endif()

set(        STMMSWAPPERXML_LIBRARIES "")
list(APPEND STMMSWAPPERXML_LIBRARIES "${STMMI_LIB_FILE}")
list(APPEND STMMSWAPPERXML_LIBRARIES "${STMMSWAPPERXML_EXTRA_LIBRARIES}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    DefineAsSecondaryTarget(stmm-swapper-xml  ${STMMI_LIB_FILE}  "${STMMsWAPPERXML_INCLUDE_DIRS}"  "stmm-swapper" "${STMMI_TEMP_EXTERNAL_LIBRARIES}")
endif()
