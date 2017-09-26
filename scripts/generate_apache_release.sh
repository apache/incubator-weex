#!/bin/bash -eu
set -e
if [ ! -f scripts/build_from_source.sh ];then
    echo "This script must execute in project root"
    exit 1
fi

dest='apache_release_temp'

rm -rf $dest
mkdir $dest
rsync -rvv  --include-from=scripts/release_files.rules ./ $dest

#repackage
find $dest/android/sdk/src -type f -name '*.java' -exec sed -i '' 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;

mkdir -p $dest/android/sdk/src/main/java/org
mkdir -p $dest/android/sdk/src/main/java/org/apache
mv $dest/android/sdk/src/main/java/com/taobao $dest/android/sdk/src/main/java/org/apache
rm -rf $dest/android/sdk/src/main/java/com

mkdir -p $dest/android/sdk/src/test/java/org
mkdir -p $dest/android/sdk/src/test/java/org/apache
mv $dest/android/sdk/src/test/java/com/taobao $dest/android/sdk/src/test/java/org/apache
rm -rf $dest/android/sdk/src/test/java/com

#sed -i '' 's/com\/taobao\/weex/org\/apache\/weex/g' $dest/android/sdk/build.gradle

mv $dest/ios/sdk $dest/ios_sdk
rm -rf $dest/ios

mv $dest/android/sdk $dest/android_sdk
mv $dest/android/build.gradle $dest/build.gradle
rm -rf $dest/android

