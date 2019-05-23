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

# Explanation for variable:
# $1 Weex Tag of current release
# $2 Weex Tag of last release, RELEASE_NOTE.md came from the difference between $1 and $2
# $3 The name of your Github repository, whose URL should be git@github.com:apache/incubator-weex.git
# $4 The personal access token of your github Account(https://github.com/settings/tokens), which should have write priviledge to git@github.com:apache/incubator-weex.git . The personal access token is used to publish Github Release
# $5 The private key for JCenter (https://bintray.com/alibabaweex/maven/weex_sdk/), which is the distribution channel for Android

# Prepare RELEASE_NOTE.md
rm -f commit-history.log
git log --pretty=format:"%s %h" --no-merges  "$2"..HEAD > commit-history.log
sed -i '' 's/^*//' commit-history.log
sed -i '' 's/^/* /' commit-history.log
sed -i '' -e '1i \
# Detail Commit' commit-history.log
cat CHANGELOG.md commit-history.log > RELEASE_NOTE.md

# Publish source file or release to Apache SVN server
if [ ! -d "${TMPDIR}weex_release" ]
then
    svn checkout https://dist.apache.org/repos/dist/release/incubator/weex/ "${TMPDIR}weex_release"
fi
mkdir -p "${TMPDIR}weex_release/${1}" && cd "$_"
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${3}/apache-weex-incubating-${1}-${3}-src.tar.gz" -o "apache-weex-incubating-${1}-src.tar.gz"
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${3}/apache-weex-incubating-${1}-${3}-src.tar.gz.asc" -o "apache-weex-incubating-${1}-src.tar.gz.asc"
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${3}/apache-weex-incubating-${1}-${3}-src.tar.gz.sha512" -o "apache-weex-incubating-${1}-src.tar.gz.sha512"
cd ..
svn add "$1"
svn commit -m "Release ${1}"

# Publish to Github Release
npm install -g release-it
export GITHUB_TOKEN="$4"
release-it --no-npm --no-git.commit --no-git.requireCleanWorkingDir --git.tagName="$1" --git.tagAnnotation='"$(cat RELEASE_NOTE.md)"' --git.tagArgs="--cleanup=verbatim" --git.pushRepo="$3" --github.release --github.releaseName="$1" --github.releaseNotes="cat RELEASE_NOTE.md"

# Publish Android to JCenter
cd android
./gradlew clean install bintray -Pweexversion="$1" -PbintrayUser=alibabaweex -PbintrayApiKey="$5" 

# Publish iOS to Cocoapods