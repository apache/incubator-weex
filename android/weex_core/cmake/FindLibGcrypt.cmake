#.rst
# FindLibGcrypt
# -------------
#
# Finds the Libgcrypt library.
#
# This will define the following variables:
#
# ``LIBGCRYPT_FOUND``
#     True if the requested version of gcrypt was found
# ``LIBGCRYPT_VERSION``
#     The version of gcrypt that was found
# ``LIBGCRYPT_INCLUDE_DIRS``
#     The gcrypt include directories
# ``LIBGCRYPT_LIBRARIES``
#     The linker libraries needed to use the gcrypt library

# Copyright 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>
# Copyright 2016 Igalia S.L
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_program(LIBGCRYPTCONFIG_SCRIPT NAMES libgcrypt-config)
if (LIBGCRYPTCONFIG_SCRIPT)
    execute_process(
        COMMAND "${LIBGCRYPTCONFIG_SCRIPT}" --prefix
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (CONFIGSCRIPT_RESULT EQUAL 0)
        set(LIBGCRYPT_LIB_HINT "${PREFIX}/lib")
        set(LIBGCRYPT_INCLUDE_HINT "${PREFIX}/include")
    endif ()
endif ()

find_library(LIBGCRYPT_LIBRARY
    NAMES gcrypt
    HINTS ${LIBGCRYPT_LIB_HINT}
)
find_path(LIBGCRYPT_INCLUDE_DIR
    NAMES gcrypt.h
    HINTS ${LIBGCRYPT_INCLUDE_HINT}
)

if (LIBGCRYPT_INCLUDE_DIR)
    file(STRINGS ${LIBGCRYPT_INCLUDE_DIR}/gcrypt.h GCRYPT_H REGEX "^#define GCRYPT_VERSION ")
    string(REGEX REPLACE "^#define GCRYPT_VERSION \"(.*)\".*$" "\\1" LIBGCRYPT_VERSION "${GCRYPT_H}")
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LibGcrypt
    FOUND_VAR LIBGCRYPT_FOUND
    REQUIRED_VARS LIBGCRYPT_LIBRARY LIBGCRYPT_INCLUDE_DIR
    VERSION_VAR LIBGCRYPT_VERSION
)
if (LIBGCRYPT_FOUND)
    set(LIBGCRYPT_LIBRARIES ${LIBGCRYPT_LIBRARY})
    set(LIBGCRYPT_INCLUDE_DIRS ${LIBGCRYPT_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBGCRYPT_LIBRARY LIBGCRYPT_INCLUDE_DIR)

include(FeatureSummary)
set_package_properties(LibGcrypt PROPERTIES
    DESCRIPTION "A general purpose cryptographic library based on the code from GnuPG."
    URL "http://www.gnu.org/software/libgcrypt/"
)

