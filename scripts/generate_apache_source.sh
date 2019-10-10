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