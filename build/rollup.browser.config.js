import { rollup } from 'rollup'
import nodeResolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import json from 'rollup-plugin-json'
import buble from 'rollup-plugin-buble'
import postcss from 'rollup-plugin-postcss'

const banner = `var global = this
var process = { env: {}};
`

export default {
  entry: './html5/render/browser/index.js',
  dest: './dist/browser.js',
  banner,
  format: 'umd',
  moduleName: 'weex',
  plugins: [
    postcss(),
    nodeResolve({
      jsnext: true,
      main: true,
      browser: true
    }),
    commonjs(),
    json(),
    buble()
  ]
}
