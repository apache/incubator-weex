# -----------------------------------------------------------------------------
# This file is included individually from various subdirectories (JSC, WTF,
# WebCore, Weex) in order to allow scripts to build only part of WebKit.
# We want to run this file only once.
# -----------------------------------------------------------------------------
if (NOT HAS_RUN_WEEX_COMMON)
    set(HAS_RUN_WEEX_COMMON TRUE)

    # -----------------------------------------------------------------------------
    # Find common packages (used by all ports)
    # -----------------------------------------------------------------------------

    # TODO Enforce version requirement for gperf

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

    include(WeexMacros)
    include(WeexFS)
    include(WeexHelpers)
    include(WeexFeatures)

    include(OptionsCommon)
    include(Options${PORT})

    # -----------------------------------------------------------------------------
    # config.h
    # -----------------------------------------------------------------------------

    CREATE_CONFIGURATION_HEADER()
endif ()
