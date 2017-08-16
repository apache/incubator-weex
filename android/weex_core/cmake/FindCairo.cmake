# - Try to find Cairo
# Once done, this will define
#
#  CAIRO_FOUND - system has Cairo
#  CAIRO_INCLUDE_DIRS - the Cairo include directories
#  CAIRO_LIBRARIES - link these to use Cairo
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PkgConfig)
pkg_check_modules(PC_CAIRO QUIET cairo)

find_path(CAIRO_INCLUDE_DIRS
    NAMES cairo.h
    HINTS ${PC_CAIRO_INCLUDEDIR}
          ${PC_CAIRO_INCLUDE_DIRS}
          ${WEBKIT_LIBRARIES_INCLUDE_DIR}/cairo
    PATH_SUFFIXES cairo
)

find_library(CAIRO_LIBRARIES
    NAMES cairo
    HINTS ${PC_CAIRO_LIBDIR}
          ${PC_CAIRO_LIBRARY_DIRS}
          ${WEBKIT_LIBRARIES_LINK_DIR}
)

if (CAIRO_INCLUDE_DIRS)
    if (EXISTS "${CAIRO_INCLUDE_DIRS}/cairo-version.h")
        file(READ "${CAIRO_INCLUDE_DIRS}/cairo-version.h" CAIRO_VERSION_CONTENT)

        string(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MAJOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MINOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MICRO "${CMAKE_MATCH_1}")

        set(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_MICRO}")
    endif ()
endif ()

if ("${Cairo_FIND_VERSION}" VERSION_GREATER "${CAIRO_VERSION}")
    message(FATAL_ERROR "Required version (" ${Cairo_FIND_VERSION} ") is higher than found version (" ${CAIRO_VERSION} ")")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cairo REQUIRED_VARS CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES
                                        VERSION_VAR CAIRO_VERSION)

mark_as_advanced(
    CAIRO_INCLUDE_DIRS
    CAIRO_LIBRARIES
)
