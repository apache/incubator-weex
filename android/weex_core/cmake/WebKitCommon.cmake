# -----------------------------------------------------------------------------
# This file is included individually from various subdirectories (JSC, WTF,
# WebCore, WebKit) in order to allow scripts to build only part of WebKit.
# We want to run this file only once.
# -----------------------------------------------------------------------------
if (NOT HAS_RUN_WEBKIT_COMMON)
    set(HAS_RUN_WEBKIT_COMMON TRUE)

    # -----------------------------------------------------------------------------
    # Find common packages (used by all ports)
    # -----------------------------------------------------------------------------
    if (WIN32)
        list(APPEND CMAKE_PROGRAM_PATH $ENV{SystemDrive}/cygwin/bin)
    endif ()

    # TODO Enforce version requirement for gperf
    find_package(Gperf 3.0.1 REQUIRED)

    # TODO Enforce version requirement for perl
    find_package(Perl 5.10.0 REQUIRED)

    find_package(PythonInterp 2.7.0 REQUIRED)

    # We cannot check for RUBY_FOUND because it is set only when the full package is installed and
    # the only thing we need is the interpreter. Unlike Python, cmake does not provide a macro
    # for finding only the Ruby interpreter.
    find_package(Ruby 1.9)
    if (NOT RUBY_EXECUTABLE OR RUBY_VERSION VERSION_LESS 1.9)
        message(FATAL_ERROR "Ruby 1.9 or higher is required.")
    endif ()

    # -----------------------------------------------------------------------------
    # Helper macros and feature defines
    # -----------------------------------------------------------------------------

    include(WebKitMacros)
    include(WebKitFS)
    include(WebKitHelpers)
    include(WebKitFeatures)

    include(OptionsCommon)
    include(Options${PORT})

    # -----------------------------------------------------------------------------
    # config.h
    # -----------------------------------------------------------------------------

    CREATE_CONFIGURATION_HEADER()
endif ()
