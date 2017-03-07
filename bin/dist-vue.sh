#!/user/bin/env bash

echo ''
echo " => writing version number into weex-vue-render's package.json..."

base_dir=./packages/weex-vue-render

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
