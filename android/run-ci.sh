#!/bin/sh -eu
# will not exist if err
set -e
./gradlew assembleDebug :weex_sdk:testDebugUnitTest --info -PdisableCov=true -Dorg.gradle.daemon=true -Dorg.gradle.parallel=true -Dorg.gradle.jvmargs="-Xmx512m -XX:+HeapDumpOnOutOfMemoryError" -Dfile.encoding=UTF-8
echo 'Android Unit Test Done!'
#COLUMNS=2000 ps -m -o pid,rss,command > sdk/build/reports/memdump.txt

