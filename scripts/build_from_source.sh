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

npm install --production
npm run install:buildtools
npm run build:source

echo "Javascript Framework and HTML5 SDK build completed."
sleep 2

cp packages/weex-js-framework/index.min.js ios_sdk/WeexSDK/Resources/main.js
cp packages/weex-js-framework/index.min.js android_sdk/assets/main.js

gradle wrapper --gradle-version 3.3
echo 'include ":android_sdk"'>settings.gradle
./gradlew :android_sdk:assemble -PasfRelease

xcodebuild -project ios_sdk/WeexSDK.xcodeproj -target WeexSDK_MTL

echo "Weex SDK Build completed."
