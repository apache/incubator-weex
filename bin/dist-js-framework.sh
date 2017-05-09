#!/bin/bash
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

echo 'packing weex-js-framework'

config=./html5/runtime/config.js

if ! [ -e $config ]; then
  echo "build JS Framework config"
  npm run build:config
fi

cd dist/weex-js-framework

rm -Rf src
mkdir src
cp ../../.babelrc .
cp -Rf ../../html5/* src/
rm -Rf src/render/browser
rm -Rf src/test

if [ $# -gt 0 ]; then
  if ! [ -e .git ]; then
    echo "init git first"
    git init
  fi
  echo "version specified! $1"
  npm version $1
  rm -Rf .git
  version=$( grep -o -E "\"version\": \"([0-9.]+)\"" package.json | grep -o -E "[0-9.]+" )
  sed -i .tmp "s/\"framework\": \"[0-9.]*\"/\"framework\": \"${version}\"/g" package.json
  rm package.json.tmp
  sed -i .tmp "s/\"framework\": \"[0-9.]*\"/\"framework\": \"${version}\"/g" ../../package.json
  rm ../../package.json.tmp
else
  echo "please pass a <version> or patch | minor | major"
fi

cd ../../

echo 'packing finished.'
