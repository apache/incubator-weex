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

SED_CMD=sed
if [ $(uname) == 'Darwin' ]; then
  SED_CMD=/usr/bin/sed
fi

#repackage
find $dest/android/sdk -type f \( -name '*.java' -o -name 'AndroidManifest.xml' -o -name 'proguard-rules.pro' \) -exec $SED_CMD -i '' 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;
find $dest/ios/sdk -type f \( -name 'project.pbxproj' -o -name '*.h' -o -name '*.m' -o -name '*.mm' \) -exec $SED_CMD -i '' 's/com\.taobao\.weex/org\.apache\.weex/g' {} \;

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
$SED_CMD -i '' 's/\.\.\/\.\.\/weex_core/\.\.\/weex_core/g' $dest/android_sdk/build.gradle $dest/ios_sdk/WeexSDK.xcodeproj/project.pbxproj
$SED_CMD -i '' 's/\.\.\/\.\.\/pre-build/\.\.\/pre-build/g' $dest/android_sdk/build.gradle $dest/ios_sdk/WeexSDK.xcodeproj/project.pbxproj
[ -d "$dest/ios_sdk/build" ] && rm -rf "$dest/ios_sdk/build"
mv $dest/android/build.gradle $dest/build.gradle
rm -rf $dest/android $dest/ios

rm -rf $dest/android_sdk/gradle
