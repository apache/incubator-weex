# - Try to find CairoGL
# Once done, this will define
#
#  CAIROGL_FOUND - system has CairoGL
#  CAIROGL_INCLUDE_DIRS - the CairoGL include directories
#  CAIROGL_LIBRARIES - link these to use CairoGL
#
# Copyright (C) 2014 Igalia S.L.
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
pkg_check_modules(CAIROGL cairo-gl)

if (CAIROGL_FOUND)
# At the moment CairoGL does not add any extra cflags and libraries, so we can
# safely ignore CAIROGL_LIBRARIES and CAIROGL_INCLUDE_DIRS for the moment.
foreach (_component ${CairoGL_FIND_COMPONENTS})
    string(TOUPPER ${_component} _UPPER_NAME)
    string(REGEX REPLACE "-" "_" _UPPER_NAME ${_UPPER_NAME})
    pkg_check_modules(${_UPPER_NAME} ${_component})
    if (${_UPPER_NAME}_INCLUDE_DIRS)
      set(CAIROGL_INCLUDE_DIRS ${CAIROGL_INCLUDE_DIRS} ${_UPPER_NAME}_INCLUDE_DIRS)
    endif ()
    if (${_UPPER_NAME}_LIBRARIES)
      set(CAIROGL_LIBRARIES ${CAIROGL_LIBRARIES} ${_UPPER_NAME}_LIBRARIES)
    endif ()
endforeach ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CairoGL DEFAULT_MSG CAIROGL_INCLUDE_DIRS CAIROGL_LIBRARIES)
endif ()
