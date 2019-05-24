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

# This scirpt is used to publish release candidate in Apache, Github
# One may invoke this script by 
# scripts/publish_release_candidate.sh $RELEASE_CANDIDATE_PREFIX $RELEASE_CANDIDATE_SUFFIX $GIT_REMOTE

# Explanation for variable:
# $RELEASE_CANDIDATE_PREFIX, Weex release candidate prefix, like 0.24.0
# $RELEASE_CANDIDATE_SUFFIX, The release candaidate suffix, like RC3
# $GIT_REMOTE The name of your Github repository, like github-Apache whose URL should be git@github.com:apache/incubator-weex.git

git tag "$1-$2"
git push "$3" "$1-$2"

if [ ! -d "$TMPDIR""weex_release_candidate" ]
then
    svn checkout https://dist.apache.org/repos/dist/dev/incubator/weex/ "$TMPDIR""weex_release_candidate"
fi

mkdir -p "$TMPDIR""weex_release_candidate/""$1/$2"
cp "apache-weex-incubating-$1-$2-src.tar.gz" "apache-weex-incubating-$1-$2-src.tar.gz.asc" "apache-weex-incubating-$1-$2-src.tar.gz.sha512" "$TMPDIR""weex_release_candidate/""$1/$2"
cd "$TMPDIR""weex_release_candidate"
svn add "$1/$2"
svn commit -m "$4"