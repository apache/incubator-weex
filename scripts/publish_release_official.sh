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

# This scirpt is used to publish release officially in Apache, Github Tag, Github Release, Android JCenter.
# One may invoke this script by 
# scripts/publish_release_official.sh 0.24.0 RC3 0.19.0 github-Apache token1 token2
# scripts/publish_release_official.sh $RELEASE_CANDIDATE_PREFIX $RELEASE_CANDIDATE_SUFFIX $TAG_OF_LATEST_RELEASE $GIT_REMOTE $GITHUB_PERSONAL_TOKEN $JCENTER_TOKEN

# Explanation for variable:
# $RELEASE_CANDIDATE_PREFIX, Weex release candidate prefix, like 0.24.0
# $RELEASE_CANDIDATE_SUFFIX, The release candaidate suffix, like RC3
# $TAG_OF_LATEST_RELEASE  Weex tag of last release, like 0.19.0.2. RELEASE_NOTE.md came from the difference between ${RELEASE_CANDIDATE_PREFIX}-${RELEASE_CANDIDATE_SUFFIX} and $TAG_OF_LATEST_RELEASE .
# $GIT_REMOTE The name of your Github repository, like github-Apache whose URL should be git@github.com:apache/incubator-weex.git
# $GITHUB_PERSONAL_TOKEN The personal access token of your github Account(https://github.com/settings/tokens), which should have write priviledge to git@github.com:apache/incubator-weex.git . The personal access token is used to publish Github Release
# $JCENTER_TOKEN The private key for JCenter (https://bintray.com/alibabaweex/maven/weex_sdk/), which is the distribution channel for Android

# Prepare RELEASE_NOTE.md
rm -f commit-history.log
git log --pretty=format:"%s %h" --no-merges  "$3"..HEAD > commit-history.log
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
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${2}/apache-weex-incubating-${1}-${2}-src.tar.gz" -o "apache-weex-incubating-${1}-src.tar.gz"
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${2}/apache-weex-incubating-${1}-${2}-src.tar.gz.asc" -o "apache-weex-incubating-${1}-src.tar.gz.asc"
curl "https://dist.apache.org/repos/dist/dev/incubator/weex/${1}/${2}/apache-weex-incubating-${1}-${2}-src.tar.gz.sha512" -o "apache-weex-incubating-${1}-src.tar.gz.sha512"
cd ..
svn add "$1"
svn commit -m "Release ${1}"

# Publish to Github Release
git tag -a -F "RELEASE_NOTE.md" "$1"
git push "$4" "$1"
npm install -g release-it
export GITHUB_TOKEN="$5"
release-it --no-npm --no-git.commit --no-git.requireCleanWorkingDir --git.tagName="$1" --git.tagAnnotation='"$(cat RELEASE_NOTE.md)"' --git.tagArgs="--cleanup=verbatim" --git.pushRepo="$4" --github.release --github.releaseName="$1" --github.releaseNotes="cat RELEASE_NOTE.md"

# Publish Android to JCenter
cd android
./gradlew clean install bintray -PignoreVersionCheck="true"  -Pweexversion="$1" -PbintrayUser=alibabaweex -PbintrayApiKey="$6" 

# Publish iOS to Cocoapods