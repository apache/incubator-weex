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
echo "--> # buils jsc_runtime so"


../gradlew assembleRelease  -PbuildRuntimeApi=true 


echo "--> ## cp jss_runtime so"

unzip -o build/outputs/aar/weex_sdk-release.aar -d build/outputs/aar/weex_sdk-release


cp build/outputs/aar/weex_sdk-release/jni/armeabi-v7a/libweexjss.so libs/armeabi-v7a/libweexjssr.so
cp build/outputs/aar/weex_sdk-release/jni/arm64-v8a/libweexjss.so libs/arm64-v8a/libweexjssr.so
cp build/outputs/aar/weex_sdk-release/jni/x86/libweexjss.so libs/x86/libweexjssr.so


echo "--> ## cp jss_runtime Symbol so"

rm -rf unstrippedSo
mkdir unstrippedSo

cp build/unstrippedSo/armeabi-v7a_libweexjss.so unstrippedSo/armeabi-v7a_libweexjssr.so
cp build/unstrippedSo/arm64-v8a_libweexjss.so unstrippedSo/arm64-v8a_libweexjssr.so
cp build/unstrippedSo/x86_libweexjss.so unstrippedSo/x86_libweexjssr.so