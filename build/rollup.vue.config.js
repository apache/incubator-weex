import { rollup } from 'rollup'
import eslint from 'rollup-plugin-eslint'
import postcss from 'rollup-plugin-postcss'
import replace from 'rollup-plugin-replace'
import nodeResolve from 'rollup-plugin-node-resolve'
import commonjs from 'rollup-plugin-commonjs'
import buble from 'rollup-plugin-buble'

export default {
  moduleName: 'VueRenderer',
  entry: './html5/render/vue/index.js',
  dest: './dist/vue-renderer.js',
  format: 'umd',
  sourceMap: 'inline',
  plugins: [
    eslint(),
    postcss(),
    replace({
      'process.env.NODE_ENV': JSON.stringify('production'),
      'process.env.VUE_ENV': JSON.stringify('weex'),
      'process.env.NODE_DEBUG': false
    }),
    nodeResolve({
      jsnext: true,
      main: true
    }),
    commonjs(),
    buble()
  ]
}
