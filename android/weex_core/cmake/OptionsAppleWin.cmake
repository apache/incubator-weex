include(OptionsWin)

set(USE_CG 1)
set(USE_CA 1)
set(USE_CFURLCONNECTION 1)
set(USE_ICU_UNICODE 1)

# Uncomment the following line to try the Direct2D backend.
# set(USE_DIRECT2D 1)

if (${USE_DIRECT2D})
    add_definitions(-DUSE_DIRECT2D=1)
endif ()

# Warnings as errors (ignore narrowing conversions)
add_compile_options(/WX /Wv:18)
