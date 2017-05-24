#!/bin/bash -eu
if [ ! -f scripts/build_from_source.sh ];then
    echo "This script must execute in project root"
    exit 1
fi

dest='apache_release_temp'

rm -rf $dest
mkdir $dest
rsync -rvv  --include-from=scripts/release_files.rules ./ $dest

mv $dest/ios/sdk $dest/ios_sdk
rm -rf $dest/ios

mv $dest/android/sdk $dest/android_sdk
mv $dest/android/build.gradle $dest/build.gradle
rm -rf $dest/android

