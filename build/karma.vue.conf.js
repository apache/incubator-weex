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

module.exports = function (config) {
  config.set({
    frameworks: ['mocha', 'sinon-chai'],
    browsers: ['PhantomJS'],
    files: [
      // '../html5/test/render/vue/components/*.js'
      '../html5/test/render/vue/**/*.js'
    ],

    exclude: [
      '../html5/test/render/vue/helper.js',
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
