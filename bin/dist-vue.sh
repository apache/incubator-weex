#!/user/bin/env bash

echo ''
echo ''
echo 'cping files for vue dist...'

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

echo 'dist finished.'
echo ''
echo ''
