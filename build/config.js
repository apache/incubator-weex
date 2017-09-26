
/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
const path = require('path')
const json = require('rollup-plugin-json')
const eslint = require('rollup-plugin-eslint')
const replace = require('rollup-plugin-replace')
const postcss = require('rollup-plugin-postcss')
const nodeResolve = require('rollup-plugin-node-resolve')
const uglify = require('rollup-plugin-uglify')
const commonjs = require('rollup-plugin-commonjs')
const flow = require('rollup-plugin-flow-no-whitespace')
const buble = require('rollup-plugin-buble')
const subversion = require('../package.json').subversion

const frameworkBanner = `;(this.getJSFMVersion = function()`
  + `{return "${subversion.framework}"});\n`
  + `var global = this, process = { env: {} };var setTimeout = global.setTimeout;\n`

const configs = {
  'weex-js-framework': {
    moduleName: 'Weex',
    entry: absolute('html5/render/native/index.js'),
    dest: absolute('packages/weex-js-framework/index.js'),
    banner: `(this.nativeLog || function(s) {console.log(s)})`
      + `('START JS FRAMEWORK ${subversion.framework}, Build ${now()}.');\n`
      + frameworkBanner,
    format: 'umd',
    plugins: [
      nodeResolve({
        jsnext: true,
        main: true
      }),
    ]
  },
  'weex-js-runtime': {
    moduleName: 'WeexRuntime',
    entry: absolute('html5/runtime/index.js'),
    dest: absolute('packages/weex-js-runtime/index.js'),
    banner: `/* WEEX JS RUNTIME ${subversion.framework}, Build ${now()}. */\n\n`,
    format: 'umd',
    plugins: [
      nodeResolve({
        jsnext: true,
        main: true
      }),
    ]
  },
  'weex-legacy-framework': {
    moduleName: 'WeexLegacyFramework',
    entry: absolute('html5/frameworks/legacy/index.js'),
    dest: absolute('packages/weex-legacy-framework/index.js'),
    banner: `/* Weex Legacy Framework ${subversion.framework}, Build ${now()}. */\n`,
    format: 'umd',
    plugins: [
      nodeResolve({
        jsnext: true,
        main: true
      }),
    ]
  },
  'weex-vanilla-framework': {
    moduleName: 'WeexVanillaFramework',
    entry: absolute('html5/frameworks/vanilla/index.js'),
    dest: absolute('packages/weex-vanilla-framework/index.js'),
    banner: `/* Weex Vanilla Framework ${subversion.framework}, Build ${now()}. */\n`,
    format: 'umd',
    plugins: [
      nodeResolve({
        jsnext: true,
        main: true
      }),
    ]
  },
  'weex-web-render': {
    moduleName: 'Weex',
    entry: absolute('html5/render/browser/index.js'),
    dest: absolute('packages/weex-html5/index.js'),
    banner: `(this.nativeLog || function(s) {console.log(s)})`
      + `('START WEEX HTML5: ${subversion.browser}, Build ${now()}.');\n`
      + frameworkBanner,
    format: 'umd',
    plugins: [
      postcss(),
      nodeResolve({
        jsnext: true,
        main: true,
        browser: true
      })
    ]
  },
  'weex-vue-render': {
    moduleName: 'WeexVueRender',
    entry: absolute('packages/weex-vue-render/src/index.js'),
    dest: absolute('packages/weex-vue-render/dist/index.js'),
    banner:`
console.log('START WEEX VUE RENDER: ${subversion['vue-render']}, Build ${now()}.');\n\n`,
    format: 'umd',
    plugins: [
      postcss(),
      nodeResolve({
        jsnext: true,
        main: true,
        browser: true
      }),
      replace({
        'process.env.WEEX_VERSION': subversion['vue-render']
      })
    ]
  },
  'weex-vue-render-core': {
    moduleName: 'WeexVueRenderCore',
    entry: absolute('packages/weex-vue-render/src/index.core.js'),
    dest: absolute('packages/weex-vue-render/dist/index.core.js'),
    banner:`
console.log('START WEEX VUE RENDER CORE: ${subversion['vue-render']}, Build ${now()}.');\n\n`,
    format: 'umd',
    plugins: [
      postcss(),
      nodeResolve({
        jsnext: true,
        main: true,
        browser: true
      }),
      replace({
        'process.env.WEEX_VERSION': subversion['vue-render']
      })
    ]
  },
  'weex-vue-render-plugins': {
    format: 'umd',
    plugins: [
      postcss(),
      nodeResolve({
        jsnext: true,
        main: true,
        browser: true
      })
    ]
  }
}

function getConfig (name, minify) {
  const opt = configs[name]
  const config = {
    moduleName: opt.moduleName,
    entry: opt.entry,
    dest: minify ? opt.dest && opt.dest.replace(/\.js$/, '.min.js') : opt.dest,
    format: opt.format,
    banner: opt.banner,
    plugins: opt.plugins.concat([
      json(),
      replace({
        'process.env.VIEWPORT_WIDTH': 750,
        'process.env.NODE_ENV': JSON.stringify(minify ? 'production' : 'development'),
        'process.env.VUE_ENV': JSON.stringify('WEEX'),
        'process.env.NODE_DEBUG': false
      }),
      commonjs(),
      flow(/*{ pretty: true }*/),
      buble()
    ])
  }

  if (minify) {
    config.plugins.push(uglify())
  }
  else {
    /**
     * rollup-plugin-flow will cause soucemap problem.
     * use rollup-plugin-flow-no-whitespace can fixe this.
     */
    config.sourceMap = 'inline'
    config.plugins.unshift(eslint({ exclude: ['**/*.json', '**/*.css'] }))
  }

  return config
}

// get the absolute path
function absolute (str) {
  return path.resolve(__dirname, '..', str)
}

function now () {
  const time = Date.now() - (new Date()).getTimezoneOffset() * 60000
  return (new Date(time)).toISOString().replace('T', ' ').substring(0, 16)
}

module.exports = getConfig
