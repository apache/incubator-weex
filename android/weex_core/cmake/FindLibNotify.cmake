# - Try to find LibNotify
# This module defines the following variables:
#
#  LIBNOTIFY_FOUND - LibNotify was found
#  LIBNOTIFY_INCLUDE_DIRS - the LibNotify include directories
#  LIBNOTIFY_LIBRARIES - link these to use LibNotify
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
# Copyright (C) 2014 Collabora Ltd.
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
pkg_check_modules(LIBNOTIFY QUIET libnotify)

find_path(LIBNOTIFY_INCLUDE_DIRS
    NAMES notify.h
    HINTS ${LIBNOTIFY_INCLUDEDIR}
          ${LIBNOTIFY_INCLUDE_DIRS}
    PATH_SUFFIXES libnotify
)

find_library(LIBNOTIFY_LIBRARIES
    NAMES notify
    HINTS ${LIBNOTIFY_LIBDIR}
          ${LIBNOTIFY_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibNotify REQUIRED_VARS LIBNOTIFY_INCLUDE_DIRS LIBNOTIFY_LIBRARIES
                                            VERSION_VAR   LIBNOTIFY_VERSION)

mark_as_advanced(
    LIBNOTIFY_INCLUDE_DIRS
    LIBNOTIFY_LIBRARIES
)
