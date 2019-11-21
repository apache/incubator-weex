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
# scripts/build_from_source.sh 

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
cd android/
if [ ! -d "gradle" ]
then
    gradle wrapper --gradle-version 4.10.1
fi

./gradlew :weex_sdk:clean :weex_sdk:assembleRelease -PignoreVersionCheck="true" -PbuildRuntimeApi=true -PapachePackageName="true"
cd ..

# Build iOS sdk
xcodebuild -project ios/sdk/WeexSDK.xcodeproj -scheme WeexSDK_MTL

echo "Weex SDK Build completed."
