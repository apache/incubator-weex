#!/bin/bash -eu
set -e
if [ ! -f scripts/build_from_source.sh ]; then
    echo "This script must execute in project root"
    exit 1
fi

dest='apache_release_temp'

rm -rf $dest
mkdir $dest
rsync -r --include-from=scripts/release_files.rules ./ $dest

#repackage
find $dest/android/sdk -type f \( -name '*.java' -o -name 'AndroidManifest.xml' -o -name 'proguard-rules.pro' \) -exec sed -i 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;
find $dest/ios/sdk -type f \( -name 'project.pbxproj' -o -name '*.h' -o -name '*.m' -o -name '*.mm' \) -exec sed -i 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;

mkdir -p $dest/android/sdk/src/main/java/org
mkdir -p $dest/android/sdk/src/main/java/org/apache
mv $dest/android/sdk/src/main/java/com/taobao $dest/android/sdk/src/main/java/org/apache
rm -rf $dest/android/sdk/src/main/java/com

mkdir -p $dest/android/sdk/src/test/java/org
mkdir -p $dest/android/sdk/src/test/java/org/apache
mv $dest/android/sdk/src/test/java/com/taobao $dest/android/sdk/src/test/java/org/apache
rm -rf $dest/android/sdk/src/test/java/com

mv $dest/ios/sdk $dest/ios_sdk
mv $dest/android/sdk $dest/android_sdk
sed -i 's/\.\.\/\.\.\/weex_core/\.\.\/weex_core/g' $dest/android_sdk/build.gradle
sed -i 's/\.\.\/\.\.\/pre-build/\.\.\/pre-build/g' $dest/android_sdk/build.gradle $dest/ios_sdk/WeexSDK.xcodeproj/project.pbxproj
mv $dest/android/build.gradle $dest/build.gradle
rm -rf $dest/android $dest/ios

