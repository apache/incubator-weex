#!/bin/bash -eu
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# This script is used to build weex from source, One may invoke this script by 
# scripts/build_from_source.sh $NDK13_dir $NDK_16dir

set -e
if [ ! -f scripts/build_from_source.sh ];then
    echo "This script must be executed from project root."
    exit 1
fi

echo "Build Weex SDK from source...."

npm install
npm run build:jsfm
npm run build:polyfill
npm run build:rax

echo "Weex JS Framework build completed."
sleep 2

# Copy built JS resources to their destination.
[ -d pre-build ] || mkdir pre-build
cp dist/weex-js-framework.min.js pre-build/native-bundle-main.js
cp dist/weex-js-framework.min.js pre-build/weex-main-jsfm.js
cp dist/weex-polyfill.min.js pre-build/weex-polyfill.js
cp dist/weex-rax.min.js pre-build/weex-rax-api.js

# Build android_sdk
if [ -d "android_sdk" ]
then
    git clone -b 0.25 --depth=1 git@github.com:alibaba/weex_js_engine.git
    export ANDROID_NDK=$1
    export PATH=$1:$PATH
    cd weex_js_engine/

    sh build.jsc.sh
    cp libWTF.so libJavaScriptCore.so ../android_sdk/libs/armeabi-v7a
    mkdir -p ../weex_core/Source/libs/armeabi-v7a
    cp libWTF.so libJavaScriptCore.so ../weex_core/Source/libs/armeabi-v7a
    rm -rf build32 libWTF.so libJavaScriptCore.so

    sh build.jsc.sh -t arm64
    cp libWTF.so libJavaScriptCore.so ../android_sdk/libs/arm64-v8a
    mkdir -p ../weex_core/Source/libs/arm64-v8a
    cp libWTF.so libJavaScriptCore.so ../weex_core/Source/libs/arm64-v8a
    rm -rf build64 libWTF.so libJavaScriptCore.so

    sh build.jsc.sh -t x86
    cp libWTF.so libJavaScriptCore.so ../android_sdk/libs/x86
    mkdir -p ../weex_core/Source/libs/x86
    cp libWTF.so libJavaScriptCore.so ../weex_core/Source/libs/x86
    rm -rf build32 libWTF.so libJavaScriptCore.so

    cd ..

    gradle wrapper --gradle-version 4.10.1
    echo 'include ":android_sdk"'>settings.gradle
    echo "ndk.dir=$1">local.properties

    ./gradlew :android_sdk:clean :android_sdk:assembleRelease -PignoreVersionCheck="true" -PbuildRuntimeApi=true

    cp android_sdk/.externalNativeBuild/cmake/release/armeabi-v7a/Source/android/jsengine/libweexjsb.so android_sdk/libs/armeabi-v7a
    cp android_sdk/.externalNativeBuild/cmake/release/armeabi-v7a/Source/android/jsengine/libweexjst.so android_sdk/libs/armeabi-v7a
    cp android_sdk/.externalNativeBuild/cmake/release/arm64-v8a/Source/android/jsengine/libweexjsb.so android_sdk/libs/armeabi
    cp android_sdk/.externalNativeBuild/cmake/release/arm64-v8a/Source/android/jsengine/libweexjst.so android_sdk/libs/armeabi
    cp android_sdk/.externalNativeBuild/cmake/release/x86/Source/android/jsengine/libweexjsb.so android_sdk/libs/x86
    cp android_sdk/.externalNativeBuild/cmake/release/x86/Source/android/jsengine/libweexjst.so android_sdk/libs/x86

else
    cd android/
    ./gradlew :playground:clean :playground:assemble
    cd ..
fi

# Build iOS sdk
if [ -d "ios_sdk" ]
then
    xcodebuild -project ios_sdk/WeexSDK.xcodeproj -scheme WeexSDK_MTL
else
    cd ios
    xcodebuild -project sdk/WeexSDK.xcodeproj -scheme WeexSDK_MTL
    cd ..
fi

echo "Weex SDK Build completed."
