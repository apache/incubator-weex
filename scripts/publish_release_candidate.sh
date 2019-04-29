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

git tag "$1-$2"
git push "$3" "$1-$2" 
cd $TMPDIR
svn checkout https://dist.apache.org/repos/dist/dev/incubator/weex/ $TMPDIR
mkdir -p "$TMPDIR/$1/$2"
cp "apache-weex-incubating-$1-$2-src.tar.gz" "apache-weex-incubating-$1-$2-src.tar.gz.asc" "apache-weex-incubating-$1-$2-src.tar.gz.sha512" "$TMPDIR/weex/$1/$2"
cd "$TMPDIR/weex"
svn add "apache-weex-incubating-$1-$2-src.tar.gz" "apache-weex-incubating-$1-$2-src.tar.gz.asc" "apache-weex-incubating-$1-$2-src.tar.gz.sha512"
svn commit -m "$4"