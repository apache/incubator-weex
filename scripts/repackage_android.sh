#!/bin/bash -eu

#repackage com.taobao to org.apache

#change package declare
find android/sdk/src -type f -name '*.java' -exec sed -i '' 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;

mkdir -p android/sdk/src/main/java/org
mkdir -p android/sdk/src/main/java/org/apache

mv android/sdk/src/main/java/com/taobao/* android/sdk/src/main/java/org/apache/

mkdir -p android/sdk/src/test/java/org
mkdir -p android/sdk/src/test/java/org/apache
mv android/sdk/src/test/java/com/taobao/* android/sdk/src/test/java/org/apache/


sed -i '' 's/com\/taobao\/weex/org\/apache\/weex/g' android/sdk/build.gradle

# find $1 -type f -name '*.java' -exec echo {} \;
