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
import fs from "fs";
import path from 'path';
import GitHubApi from 'github';
import parseDiff from 'parse-diff';
import shell from "shelljs";

const type_unknown = 0;
const type_ios_sdk = 1;
const type_android_sdk = 2;
const type_ios_test = 3;
const type_android_test = 4;
const type_jsfm = 5;
const type_jsfm_test = 6;
const type_ui_test = 8;

const getFileType = file => {
  if (file.match(/WeexSDK\/Sources\/.+\.(m|h|mm)/)) {
    return type_ios_sdk;
  } else if (file.match(/WeexSDKTests\//)) {
    return type_ios_test;
  } else if (file.match(/android\/sdk\/src\/test\/.+\.java/)) {
    return type_android_test;
  } else if (file.match(/android\/sdk\/src\/main\/java\/.+\.java/)) {
    return type_android_sdk;
  } else if (
    file.match(/html5\/(shared|frameworks|render|runtime|services)\/.+\.js/)
  ) {
    return type_jsfm;
  } else if (file.match(/html5\/test\/.+\.js/)) {
    return type_jsfm_test;
  } else if (file.match(/test\/scripts\/.+\.js/) || file.match(/test\/pages\/.+\.vue/)){
    return type_ui_test
  } else {
    return type_unknown
  }
}

function checkJsfmFile(file){
  var type = getFileType(file);
  return type == type_jsfm || type == type_jsfm_test;
}

var hasJsfmFile = false;

if (!hasJsfmFile && danger.git.created_files) {
  danger.git.created_files.some(file => {
    var f = checkJsfmFile(file);
    if(f){
      hasJsfmFile =f;
    }
    return f;
  });
}
if (!hasJsfmFile && danger.git.modified_files) {
  danger.git.modified_files.some(file => {
    var f = checkJsfmFile(file);
    if(f){
      hasJsfmFile =f;
    }
    return f;
  });
}
if (!hasJsfmFile && danger.git.deleted_files) {
  danger.git.deleted_files.some(file => {
    var f = checkJsfmFile(file);
    if(f){
      hasJsfmFile =f;
    }
    return f;
  });
}
console.log('-----------------------------hasJsfmFile-----------------------------:'+hasJsfmFile);
if(hasJsfmFile){
  var runTestCmd='source ~/.bash_profile; '
    + 'npm run build && npm run test'
  var runSuccess = shell.exec(runTestCmd,{ async: false, timeout: 8 * 60 * 1000, maxBuffer: 200 * 1024 * 1024 }).code == 0;
  if(!runSuccess){
    fail("jsfm run test failed!");
  }
}else{
  console.log('has no jsfm file changed.');
}