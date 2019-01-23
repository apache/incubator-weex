#!/bin/bash
CURRENT_PATH=$(cd `dirname $0`; pwd)
ROOT_WEEX_JAVA_PATH=$CURRENT_PATH"/../../../../../android/sdk/src/main/java/"
WEEX_OUTPUT_DIR=$CURRENT_PATH"/jniheader/"
WEEX_GEN_FILE=$CURRENT_PATH"/jni_generator.py"
while read line
do
	file_name=${line}
    jni_file_name=${file_name%.*}"_jni.h"
    input_file=$ROOT_WEEX_JAVA_PATH$line
    output_file=$WEEX_OUTPUT_DIR$jni_file_name
    python $WEEX_GEN_FILE $input_file $output_file
done < $CURRENT_PATH"/jni_files"
