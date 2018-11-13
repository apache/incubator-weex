/**
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

#!/bin/bash
CURRENT_PATH=$(cd `dirname $0`; pwd)
ROOT_WEEX_JAVA_PATH=$CURRENT_PATH"/../../../../android/sdk/src/main/java/"
WEEX_OUTPUT_DIR=$CURRENT_PATH"/jniheader/"
WEEX_GEN_FILE=$CURRENT_PATH"/jni_generator.py"
while read line
do
	file_name=${line##*/}
    jni_file_name=${file_name%.*}"_jni.h"
    input_file=$ROOT_WEEX_JAVA_PATH$line
    output_file=$WEEX_OUTPUT_DIR$jni_file_name
    python $WEEX_GEN_FILE $input_file $output_file
done < $CURRENT_PATH"/jni_files"