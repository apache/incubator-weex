# Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies).
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PkgConfig)
pkg_check_modules(PC_FREETYPE2 REQUIRED freetype2)

find_path(FREETYPE2_HEADER_DIR
    NAMES freetype.h
    HINTS ${PC_FREETYPE2_INCLUDE_DIRS}
          ${PC_FREETYPE2_INCLUDEDIR}
    PATH_SUFFIXES freetype
)

find_path(FREETYPE2_ROOT_INCLUDE_DIR
    NAMES freetype/freetype.h
    HINTS ${PC_FREETYPE2_INCLUDE_DIRS}
          ${PC_FREETYPE2_INCLUDEDIR}
)

find_library(FREETYPE2_LIBRARIES
    NAMES freetype
    HINTS ${PC_FREETYPE2_LIBDIR}
          ${PC_FREETYPE2_LIBRARY_DIRS}
)

set(FREETYPE2_INCLUDE_DIRS ${FREETYPE2_HEADER_DIR})

# Since Freetype 2.5.1 there is no freetype/freetype.h, so this variable can be null
if (FREETYPE2_ROOT_INCLUDE_DIR)
    list(APPEND FREETYPE2_INCLUDE_DIRS ${FREETYPE2_ROOT_INCLUDE_DIR})
endif ()

# Inspired in the original FindFreetype.cmake
if (FREETYPE2_HEADER_DIR AND EXISTS "${FREETYPE2_HEADER_DIR}/freetype.h")
    file(STRINGS "${FREETYPE2_HEADER_DIR}/freetype.h" freetype_version_str
         REGEX "^#[\t ]*define[\t ]+FREETYPE_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

    unset(FREETYPE2_VERSION_STRING)
    foreach (VPART MAJOR MINOR PATCH)
        foreach (VLINE ${freetype_version_str})
            if (VLINE MATCHES "^#[\t ]*define[\t ]+FREETYPE_${VPART}")
                string(REGEX REPLACE "^#[\t ]*define[\t ]+FREETYPE_${VPART}[\t ]+([0-9]+)$" "\\1"
                       FREETYPE2_VERSION_PART "${VLINE}")
                if (FREETYPE2_VERSION_STRING)
                    set(FREETYPE2_VERSION_STRING "${FREETYPE2_VERSION_STRING}.${FREETYPE2_VERSION_PART}")
                else ()
                    set(FREETYPE2_VERSION_STRING "${FREETYPE2_VERSION_PART}")
                endif ()
                unset(FREETYPE2_VERSION_PART)
            endif ()
        endforeach ()
    endforeach ()
endif ()

set(VERSION_OK TRUE)
if (FREETYPE2_VERSION_STRING)
    if (PC_FREETYPE2_FIND_VERSION_EXACT)
        if (NOT("${Freetype2_FIND_VERSION}" VERSION_EQUAL "${FREETYPE2_VERSION_STRING}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${FREETYPE2_VERSION_STRING}" VERSION_LESS "${Freetype2_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype2 DEFAULT_MSG FREETYPE2_INCLUDE_DIRS FREETYPE2_LIBRARIES VERSION_OK)

mark_as_advanced(
    FREETYPE2_HEADER_DIR
    FREETYPE2_LIBRARIES
    FREETYPE2_ROOT_INCLUDE_DIR
)
