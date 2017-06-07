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
# echo ' => cping files for vue dist...'

base_dir=./packages/weex-vue-render
src_dir=./html5
vue_dir=${src_dir}/render/vue

# browser_dir=${src_dir}/render/browser
# shared_dir=${src_dir}/shared

# dist_dir=./packages/weex-vue-render/src

# rm -rf ${dist_dir}

# mkdir $dist_dir
# mkdir $dist_dir/render

# cp -fR ${browser_dir} ${dist_dir}/render/
# cp -fR ${vue_dir} ${dist_dir}/render/
# cp -fR ${shared_dir} ${dist_dir}/

echo " => cp READEME.md to dist path..."

cp -f ${vue_dir}/README.md ${base_dir}/

echo " => writing version number into weex-vue-render's package.json..."

# get version of weex-html5 from subversion of main package.json.
pkg=./package.json
dist_pkg=${base_dir}/package.json
version=$( grep -o -E "\"vue-render\": \"([0-9.]+)\"" ${pkg} | grep -o -E "[0-9.]+" )
echo " => version:" ${version}

# update package.json for weex-html5 package.
cat ${dist_pkg} | sed "s/\"version\": \"[0-9.]*\"/\"version\": \"${version}\"/g" > ${base_dir}/tmp.json
# cat ${dist_pkg} | sed "s/\"version\": \"[0-9.]*\"/\"version\": \"${version}\"/g;s/\"transformer\": \"[>=< 0-9.]*\"/\"transformer\": \"${transformerVersion}\"/g" > ${base_dir}/tmp.json
rm ${dist_pkg}
mv ${base_dir}/tmp.json ${dist_pkg}

echo ' => writing finished.'
echo ''
