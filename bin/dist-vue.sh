#!/user/bin/env bash

echo ''
echo ' => cping files for vue dist...'

base_dir=./packages/weex-vue-render
src_dir=./html5
vue_dir=${src_dir}/render/vue
browser_dir=${src_dir}/render/browser
shared_dir=${src_dir}/shared

dist_dir=./packages/weex-vue-render/src

rm -rf ${dist_dir}

mkdir $dist_dir
mkdir $dist_dir/render

cp -fR ${browser_dir} ${dist_dir}/render/
cp -fR ${vue_dir} ${dist_dir}/render/
cp -fR ${shared_dir} ${dist_dir}/

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
