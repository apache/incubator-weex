#!/bin/bash
if [ $# -eq 0 ]; then
    TARGET=arm
else
    TARGET=$1
fi

export NDK_ROOT=`dirname $(which ndk-build)`
echo $NDK_ROOT
export APILEVEL=21
HOST=$(uname -s)-$(uname -m)
export HOST=$(echo $HOST | tr '[:upper:]' '[:lower:]')
export TOOLCHAIN_PATH=$NDK_ROOT/toolchains/llvm/prebuilt/$HOST/bin

if [ $TARGET = "arm" ]; then
    export TARGET
    export TARGET_ABI=armeabi-v7a
    export CLANG_CFLAGS='-target armv7-linux-android -mthumb -funwind-tables'
    export TARGET_WORD_BITS=32
    export GCC_PATH=`dirname $(find $NDK_ROOT/toolchains $NDK_ROOT/build  -name 'arm*' -name '*-g++' | sort  -r| head -n 1)`/../
    export BINARY_PATH=$GCC_PATH/arm-linux-androideabi/bin
    export DEFAULT_LIBRARY="-lc++_static -lc++abi -lunwind -landroid_support"
    export WTF_CPU=ARM
    echo "building arm $BINARY_PATH"
elif [ $TARGET = "arm64" ]; then
    export TARGET
    export TARGET_ABI=arm64-v8a
    export CLANG_CFLAGS='-target aarch64-linux-android'
    export TARGET_WORD_BITS=64
    export GCC_PATH=`dirname $(find $NDK_ROOT/toolchains $NDK_ROOT/build  -name 'aarch64*' -name '*-g++' | sort  -r| head -n 1)`/../
    export BINARY_PATH=$GCC_PATH/aarch64-linux-android/bin
    export DEFAULT_LIBRARY="-lc++_static -lc++abi -landroid_support"
    export WTF_CPU=ARM64
    echo 'building arm64'
elif [ $TARGET = "x86" ]; then
    export TARGET
    export TARGET_ABI=x86
    export CLANG_CFLAGS='-target i686-linux-android -fPIC'
    export TARGET_WORD_BITS=32
    export GCC_PATH=`dirname $(find $NDK_ROOT/toolchains $NDK_ROOT/build  -name 'i686*' -name '*-g++' | sort  -r| head -n 1)`/../
    export BINARY_PATH=$GCC_PATH/i686-linux-android/bin
    export DEFAULT_LIBRARY="-lc++_static -lc++abi -landroid_support"
    export WTF_CPU=X86
    echo 'building x86'
else
    echo "unknown target" 1>&2
    exit 1
fi

mkdir -p build${TARGET_WORD_BITS}
cd build${TARGET_WORD_BITS}
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && cmake --build .
