import { rollup } from 'rollup'
import nodeResolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import json from 'rollup-plugin-json'
import buble from 'rollup-plugin-buble'
import postcss from 'rollup-plugin-postcss'

export default {
  entry: './html5/render/browser/index.js',
  dest: './dist/browser.common.js',
  format: 'cjs',
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
