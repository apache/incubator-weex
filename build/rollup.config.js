import { rollup } from 'rollup'
import nodeResolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import json from 'rollup-plugin-json'
import buble from 'rollup-plugin-buble'

const banner = `var global = this
var process = { env: {}}
`

export default {
  entry: './html5/render/native/index.js',
  dest: './dist/native.js',
  banner,
  plugins: [ nodeResolve({ jsnext: true, main: true }), commonjs(), json(), buble() ]
}
