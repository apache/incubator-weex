# - Try to Find OpenGL
# Once done, this will define
#
#  OPENGL_FOUND - system has OpenGL installed.
#  OPENGL_INCLUDE_DIRS - directories which contain the OpenGL headers.
#  OPENGL_LIBRARIES - libraries required to link against OpenGL.
#  OPENGL_DEFINITIONS - Compiler switches required for using OpenGL.
#
# Copyright (C) 2015 Igalia S.L.
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

pkg_check_modules(PC_OPENGL gl)

if (PC_OPENGL_FOUND)
    set(OPENGL_DEFINITIONS ${PC_OPENGL_CFLAGS_OTHER})
endif ()

find_path(OPENGL_INCLUDE_DIRS NAMES GL/gl.h
    HINTS ${PC_OPENGL_INCLUDEDIR} ${PC_OPENGL_INCLUDE_DIRS}
)

set(OPENGL_NAMES ${OPENGL_NAMES} gl GL)
find_library(OPENGL_LIBRARIES NAMES ${OPENGL_NAMES}
    HINTS ${PC_OPENGL_LIBDIR} ${PC_OPENGL_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENGL DEFAULT_MSG OPENGL_INCLUDE_DIRS OPENGL_LIBRARIES)

mark_as_advanced(OPENGL_INCLUDE_DIRS OPENGL_LIBRARIES)

if (OPENGL_FOUND)
    # We don't use find_package for GLX because it is part of -lGL, unlike EGL. We need to
    # have OPENGL_INCLUDE_DIRS as part of the directories check_include_files() looks for in
    # case OpenGL is installed into a non-standard location.
    include(CMakePushCheckState)
    CMAKE_PUSH_CHECK_STATE()
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OPENGL_INCLUDE_DIRS})
    include(CheckIncludeFiles)
    check_include_files("GL/glx.h" GLX_FOUND)
    CMAKE_POP_CHECK_STATE()
endif ()
