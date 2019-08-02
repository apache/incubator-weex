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

import { danger } from "danger";
import fs from "fs";
import path from 'path';
import GitHubApi from 'github';
import parseDiff from 'parse-diff';
import shell from "shelljs";

const type_unknown = 0;
const type_c = 1;
const type_android = 2;

const getFileType = file => {
    if (file.match(/.+\.(m|h|mm|cpp|cc)/)) {
        return type_c;
    } else if (file.match(/android/)) {
        return type_android;
    } else {
        return type_unknown;
    }
}

var hasCFile = false;
var hasAndroidFile = false;

function check(file_type) {
    var has_file_type = false;
    if (!has_file_type && danger.git.created_files) {
        danger.git.created_files.some(file => {
            var f = (getFileType(file) == file_type)
            if (f) {
                has_file_type = f;
            }
            return f;
        });
    }

    if (!has_file_type && danger.git.modified_files) {
        danger.git.modified_files.some(file => {
            var f = (getFileType(file) == file_type)
            if (f) {
                has_file_type = f;
            }
            return f;
        });
    }

    if (!has_file_type && danger.git.deleted_files) {
        danger.git.deleted_files.some(file => {
            var f = (getFileType(file) == file_type)
            if (f) {
                has_file_type = f;
            }
            return f;
        });
    }

    return has_file_type
}

hasCFile = check(type_c)
hasAndroidFile = check(type_android)

var output_str = ""
if (hasCFile) {
    output_str += 'hasCFile\n'
}
if (hasAndroidFile) {
    output_str += 'hasAndroidFile\n'
}
console.log(output_str)
