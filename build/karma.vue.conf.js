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
var rollupConfig = require('./config')('weex-vue-render', true)
var path = require('path')
var flow = require('rollup-plugin-flow')

function absolute (pa) {
  return path.resolve(__dirname, pa)
}

var removeConfigs = ['entry', 'dest', 'banner']
var removePlugins = ['eslint', 'uglify', 'flow-remove-type']

removeConfigs.forEach(cfg => {
  delete rollupConfig[cfg]
})

var plugins = rollupConfig.plugins.slice()
rollupConfig.plugins = []

for (var i = 0, l = plugins.length; i < l; i++) {
  var plg = plugins[i]
  var isRemove = false
  if (removePlugins.length <= 0) {
    rollupConfig.plugins.push(plg)
  }
  removePlugins.forEach(rp => {
    if (plg.name === rp) {
      isRemove = true
    }
  })
  if (!isRemove) { rollupConfig.plugins.push(plg) }
}

/**
 * rollup-plugin-flow-no-whiitespace cause fatial error.
 * use rollup-plugin-flow instead, since no soucemap needed, it's acceptable for extra whitespaces.
 */
rollupConfig.plugins.splice(-2, 1, flow())

rollupConfig.format = 'iife'
rollupConfig.sourceMap = 'inline'
rollupConfig.intro = `describe('ignore inject function from postcss', function () {
    it('ignore', function () {
        var shouldBe = 'test'
        var expected = __$styleInject('.body{}',shouldBe)
        expect(shouldBe).to.be.equal(expected)
      })
    })`

module.exports = function (config) {
  config.set({
    frameworks: ['mocha', 'sinon-chai'],
    browsers: ['PhantomJS'],
    files: [
      '../html5/test/render/vue/utils/*.js',
      '../html5/test/render/vue/core/*.js',
      '../html5/test/render/vue/!(utils|core)/*.js'
    ],
    exclude: [
      '../html5/test/render/vue/helper/*.js',
      '../html5/test/render/vue/vender/**/*.js',
      '../html5/test/render/vue/data/**/*.js'
    ],

    // singleRun: false,
    singleRun: true,

    reporters: ['mocha', 'coverage'],
    coverageReporter: {
      reporters: [
        { type: 'html', dir: absolute('../coverage'), subdir: 'vue-renderer' },
        { type: 'text-summary', dir: absolute('../coverage'), subdir: 'vue-renderer' }
      ]
    },
    browserDisconnectTimeout:10000,
    preprocessors: {
      '../html5/test/**/*.js': ['rollup'],
      '../html5/test/**/!(components|examples|core)/*.js': ['rollup', 'coverage']
    },
    rollupPreprocessor: rollupConfig,

    plugins: [
      'karma-coverage',
      'karma-mocha',
      'karma-mocha-reporter',
      'karma-phantomjs-launcher',
      'karma-rollup-preprocessor',
      'karma-sinon-chai'
      // 'karma-sourcemap-loader',
      // 'karma-webpack'
    ]
  })
}
