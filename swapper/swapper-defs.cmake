# File: swapper/swapper-defs.cmake

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

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(SWAPPER_MAJOR_VERSION 0)
set(SWAPPER_MINOR_VERSION 25) # !-U-!
set(SWAPPER_VERSION "${SWAPPER_MAJOR_VERSION}.${SWAPPER_MINOR_VERSION}.0")

# required stmm-swapper-xml version
set(SWAPPER_REQ_STMM_SWAPPER_XML_MAJOR_VERSION 0)
set(SWAPPER_REQ_STMM_SWAPPER_XML_MINOR_VERSION 25) # !-U-!
set(SWAPPER_REQ_STMM_SWAPPER_XML_VERSION "${SWAPPER_REQ_STMM_SWAPPER_XML_MAJOR_VERSION}.${SWAPPER_REQ_STMM_SWAPPER_XML_MINOR_VERSION}")

# required stmm-games-xml-gtk version
set(SWAPPER_REQ_STMM_GAMES_XML_GTK_MAJOR_VERSION 0)
set(SWAPPER_REQ_STMM_GAMES_XML_GTK_MINOR_VERSION 25) # !-U-!
set(SWAPPER_REQ_STMM_GAMES_XML_GTK_VERSION "${SWAPPER_REQ_STMM_GAMES_XML_GTK_MAJOR_VERSION}.${SWAPPER_REQ_STMM_GAMES_XML_GTK_MINOR_VERSION}")

# required stmm-input-gtk-dm version
set(SWAPPER_REQ_STMM_INPUT_GTK_DM_MAJOR_VERSION 0)
set(SWAPPER_REQ_STMM_INPUT_GTK_DM_MINOR_VERSION 14) # !-U-!
set(SWAPPER_REQ_STMM_INPUT_GTK_DM_VERSION "${SWAPPER_REQ_STMM_INPUT_GTK_DM_MAJOR_VERSION}.${SWAPPER_REQ_STMM_INPUT_GTK_DM_MINOR_VERSION}")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(STMMGAMESXMLGTK  REQUIRED  stmm-games-xml-gtk>=${SWAPPER_REQ_STMM_GAMES_XML_GTK_VERSION})
    pkg_check_modules(STMMINPUTGTKDM   REQUIRED  stmm-input-gtk-dm>=${SWAPPER_REQ_STMM_INPUT_GTK_DM_VERSION})
endif()

include("${PROJECT_SOURCE_DIR}/../libstmm-swapper-xml/stmm-swapper-xml-defs.cmake")

# include dirs
set(        SWAPPER_EXTRA_INCLUDE_DIRS  "")
list(APPEND SWAPPER_EXTRA_INCLUDE_DIRS  "${STMMSWAPPERXML_INCLUDE_DIRS}")
set(        SWAPPER_EXTRA_INCLUDE_SDIRS "")
list(APPEND SWAPPER_EXTRA_INCLUDE_SDIRS "${STMMSWAPPERXML_INCLUDE_SDIRS}")
list(APPEND SWAPPER_EXTRA_INCLUDE_SDIRS "${STMMGAMESXMLGTK_INCLUDE_DIRS}")
list(APPEND SWAPPER_EXTRA_INCLUDE_SDIRS "${STMMINPUTGTKDM_INCLUDE_DIRS}")

# libs
set(        SWAPPER_EXTRA_LIBRARIES     "")
list(APPEND SWAPPER_EXTRA_LIBRARIES     "${STMMSWAPPERXML_LIBRARIES}")
list(APPEND SWAPPER_EXTRA_LIBRARIES     "${STMMGAMESXMLGTK_LIBRARIES}")
list(APPEND SWAPPER_EXTRA_LIBRARIES     "${STMMINPUTGTKDM_LIBRARIES}")
