/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
import { schedule, danger, fail, warn, message, markdown } from "danger";

const type_unknown = 0;
const type_ios_sdk = 1;
const type_android_sdk = 2;
const type_ios_test = 3;
const type_android_test = 4;
const type_jsfm = 5;
const type_jsfm_test = 6;
const type_ui_test = 8;
const type_weex_core =9

const getFileType = file => {
  if (file.match(/WeexSDK\/Sources\/.+\.(m|h|mm)/)) {
    return type_ios_sdk;
  } else if (file.match(/WeexSDKTests\//)) {
    return type_ios_test;
  } else if (file.match(/android\/sdk\//)) {
    return type_android_sdk;
  } else if (file.match(/html5\/(shared|frameworks|render|runtime|services)\/.+\.js/)) {
    return type_jsfm;
  } else if (file.match(/html5\/test\/.+\.js/)) {
    return type_jsfm_test;
  } else if(file.match(/test\/scripts\/.+\.js/) || file.match(/test\/pages\/.+\.vue/)){
    return type_ui_test
  } else if(file.match(/weex_core\//)){
    return type_weex_core
  }
  else{
    return type_unknown
  }
}

function checkAndroidFile(file){
  var type = getFileType(file);
  return type == type_android_test || type == type_android_sdk || type == type_jsfm || type == type_weex_core;
}

var hasAndroidFile = false;

if (!hasAndroidFile && danger.git.created_files) {
  danger.git.created_files.some(file => {
    var f = checkAndroidFile(file);
    if(f){
      hasAndroidFile =f;
    }
    return f;
  });
}
if (!hasAndroidFile && danger.git.modified_files) {
  danger.git.modified_files.some(file => {
    var f = checkAndroidFile(file);
    if(f){
      hasAndroidFile =f;
    }
    return f;
  });
}
if (!hasAndroidFile && danger.git.deleted_files) {
  danger.git.deleted_files.some(file => {
    var f = checkAndroidFile(file);
    if(f){
      hasAndroidFile =f;
    }
    return f;
  });
}
if(hasAndroidFile){
  console.log('hasAndroidFiled');
}
