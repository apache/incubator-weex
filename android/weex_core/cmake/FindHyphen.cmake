# - Try to find libhyphen
# Once done, this will define
#
#  HYPHEN_FOUND - system has libhyphen installed.
#  HYPHEN_INCLUDE_DIR - directories which contain the libhyphen headers.
#  HYPHEN_LIBRARY - libraries required to link against libhyphen.
#
# Copyright (C) 2012 Intel Corporation. All rights reserved.
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


find_path(HYPHEN_INCLUDE_DIR NAMES hyphen.h)
find_library(HYPHEN_LIBRARIES NAMES hyphen hnj)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HYPHEN DEFAULT_MSG HYPHEN_INCLUDE_DIR HYPHEN_LIBRARIES)

if (HYPHEN_INCLUDE_DIR AND HYPHEN_LIBRARIES)
    set(HYPHEN_FOUND 1)
else ()
    set(HYPHEN_FOUND 0)
endif ()

mark_as_advanced(HYPHEN_INCLUDE_DIR HYPHEN_LIBRARIES HYPHEN_FOUND)
