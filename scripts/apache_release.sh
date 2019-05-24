#!/bin/bash -eu
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# This script is used to genereate release source, signature and checksum. One may invoke this script by
# scripts/apache_release.sh $RELEASE_CANDIDATE $TAG_OF_LATEST_RELEASE

# Explanation for variable:
# $RELEASE_CANDIDATE The fullname of the release candidate, like 0.24.0-RC3
# $TAG_OF_LATEST_RELEASE  Weex tag of last release, like 0.19.0.2. RELEASE_NOTE.md came from the difference between $RELEASE_CANDIDATE and $TAG_OF_LATEST_RELEASE .

rm -f commit-history.log
git log --pretty=format:"%s %h" --no-merges  "$2"..HEAD > commit-history.log
sed -i '' 's/^*//' commit-history.log
sed -i '' 's/^/* /' commit-history.log
sed -i '' -e '1i \
# Detail Commit' commit-history.log
cat CHANGELOG.md commit-history.log > RELEASE_NOTE.md

sh scripts/generate_apache_source.sh
java -jar ./scripts/apache-rat-0.12.jar apache_release_temp > RELEASE_AUDIT.LOG
cp -r apache_release_temp apache-weex-incubating-$1-src
tar -czf "apache-weex-incubating-$1-src.tar.gz" apache-weex-incubating-$1-src/
rm -f "apache-weex-incubating-$1-src.tar.gz.asc"
gpg --armor --output "apache-weex-incubating-$1-src.tar.gz.asc" --detach-sig "apache-weex-incubating-$1-src.tar.gz"
gpg --print-md SHA512 "apache-weex-incubating-$1-src.tar.gz" > "apache-weex-incubating-$1-src.tar.gz.sha512"
rm -rf apache-weex-incubating-$1-src