#!/user/bin/env bash
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

echo ''
echo ''
echo 'packing weex-html5 for distribution...'

src_dir=./html5
base_dir=./packages/weex-html5
dist_dir=${base_dir}/dist

# get version of weex-html5 from subversion of main package.json.
pkg=./package.json
dist_pkg=${base_dir}/package.json
version=$( grep -o -E "\"browser\": \"([0-9.]+)\"" ${pkg} | grep -o -E "[0-9.]+" )
transformerVersion=$( grep -o -E "\"transformer\": \"([>=< 0-9.]+)\"" ${pkg} | grep -o -E "[>=< 0-9.]+")
echo "version:" ${version}
echo "availableTransfomer version:" ${transformerVersion}

# update package.json for weex-html5 package.
cat ${dist_pkg} | sed "s/\"version\": \"[0-9.]*\"/\"version\": \"${version}\"/g" > ${base_dir}/tmp.json
# cat ${dist_pkg} | sed "s/\"version\": \"[0-9.]*\"/\"version\": \"${version}\"/g;s/\"transformer\": \"[>=< 0-9.]*\"/\"transformer\": \"${transformerVersion}\"/g" > ${base_dir}/tmp.json
rm ${dist_pkg}
mv ${base_dir}/tmp.json ${dist_pkg}

# prepare dist directory to put weex files.
if [ -e $dist_dir ]
  then rm -rf $dist_dir
fi

mkdir $dist_dir

cp ./dist/browser.js ${dist_dir}/weex.js
cp ./dist/browser.common.js ${dist_dir}/weex.common.js
# cp ${dist_dir}/weex.js ${dist_dir}/weex.common.js
echo -e "\nmodule.exports = global.weex;" >> ${dist_dir}/weex.common.js

uglifyjs ${dist_dir}/weex.js -o ${dist_dir}/weex.min.js

# rm -rf ${base_dir}/src
# cp -fR ${src_dir} ${base_dir}/src

echo 'packing finished.'
echo ''
echo ''
