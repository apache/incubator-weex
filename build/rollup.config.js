import { rollup } from 'rollup'
import json from 'rollup-plugin-json'
import buble from 'rollup-plugin-buble'
 
export default {
  entry: './html5/render/native/index.js',
  dest: './dist/native.js',
  plugins: [ json(), buble() ]
}
