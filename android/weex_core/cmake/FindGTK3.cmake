# - Try to find GTK+ 3
# Once done, this will define
#
#  GTK3_FOUND - system has GTK+ 3.
#  GTK3_INCLUDE_DIRS - the GTK+ 3. include directories
#  GTK3_LIBRARIES - link these to use GTK+ 3.
#  GTK3_SUPPORTS_GESTURES - GTK+ supports gestures (GTK+ >= 3.14)
#  GTK3_SUPPORTS_QUARTZ - GTK+ supports Quartz backend
#  GTK3_SUPPORTS_X11 - GTK+ supports X11 backend
#  GTK3_SUPPORTS_WAYLAND - GTK+ supports Wayland backend
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
# Copyright (C) 2013, 2015 Igalia S.L.
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
pkg_check_modules(GTK3 gtk+-3.0)

set(VERSION_OK TRUE)
if (GTK3_VERSION)
    if (GTK3_FIND_VERSION_EXACT)
        if (NOT("${GTK3_FIND_VERSION}" VERSION_EQUAL "${GTK3_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${GTK3_VERSION}" VERSION_LESS "${GTK3_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

if (GTK3_VERSION AND VERSION_OK)
    pkg_check_modules(GTK3_QUARTZ gtk+-quartz-3.0)
    if ("${GTK3_QUARTZ_VERSION}" VERSION_EQUAL "${GTK3_VERSION}")
        set(GTK3_SUPPORTS_QUARTZ TRUE)
    else ()
        set(GTK3_SUPPORTS_QUARTZ FALSE)
    endif ()

    pkg_check_modules(GTK3_X11 gtk+-x11-3.0)
    if ("${GTK3_X11_VERSION}" VERSION_EQUAL "${GTK3_VERSION}")
        set(GTK3_SUPPORTS_X11 TRUE)
    else ()
        set(GTK3_SUPPORTS_X11 FALSE)
    endif ()

    pkg_check_modules(GTK3_WAYLAND gtk+-wayland-3.0)
    if ("${GTK3_WAYLAND_VERSION}" VERSION_EQUAL "${GTK3_VERSION}")
        set(GTK3_SUPPORTS_WAYLAND TRUE)
    else ()
        set(GTK3_SUPPORTS_WAYLAND FALSE)
    endif ()

    if (NOT("${GTK3_VERSION}" VERSION_LESS "3.14.0"))
        set(GTK3_SUPPORTS_GESTURES ON)
    else ()
        set(GTK3_SUPPORTS_GESTURES OFF)
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK3 DEFAULT_MSG GTK3_INCLUDE_DIRS GTK3_LIBRARIES VERSION_OK)
