import { rollup } from 'rollup'
import postcss from 'rollup-plugin-postcss'
import json from 'rollup-plugin-json'
import eslint from 'rollup-plugin-eslint'
import nodeResolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import buble from 'rollup-plugin-buble'

const banner = `var global = this
var process = { env: {}};
`

export default {
  entry: './html5/render/browser/index.js',
  dest: './dist/browser.js',
  banner,
  format: 'umd',
  moduleName: 'weex',
  sourceMap: 'inline',
  plugins: [
    postcss(),
    json(),
    eslint({
      exclude: ['./package.json', '**/*.css']
    }),
    nodeResolve({
      jsnext: true,
      main: true,
      browser: true
    }),
    commonjs(),
    buble()
  ]
}
