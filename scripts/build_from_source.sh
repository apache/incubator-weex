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

set -e
if [ ! -f scripts/build_from_source.sh ];then
    echo "This script must execute in project root"
    exit 1
fi

echo "Build Weex SDK From Source...."

npm install
npm run build:jsfm
npm run build:polyfill
npm run build:rax

echo "Weex JS Framework build completed."
sleep 2

mkdir pre-build
cp dist/weex-js-framework.min.js pre-build/native-bundle-main.js
cp dist/weex-js-framework.min.js android_sdk/assets/native-bundle-main.js
cp dist/weex-js-framework.min.js pre-build/weex-main-jsfm.js
cp dist/weex-js-framework.min.js android_sdk/assets/weex-main-jsfm.js
cp dist/weex-polyfill.min.js pre-build/weex-polyfill.js
cp dist/weex-rax.min.js pre-build/weex-rax-api.js
cp dist/weex-rax.min.js android_sdk/assets/weex-rax-api.js

gradle wrapper --gradle-version 4.4
echo 'include ":android_sdk"'>settings.gradle
./gradlew :android_sdk:assemble -PasfRelease

xcodebuild -project ios_sdk/WeexSDK.xcodeproj -target WeexSDK_MTL

echo "Weex SDK Build completed."
