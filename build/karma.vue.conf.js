var rollupConfig = require('./config')('weex-vue-render', true)
var path = require('path')

function absolute (pa) {
  return path.resolve(__dirname, pa)
}

var removeConfigs = ['entry', 'dest', 'banner']
var removePlugins = ['eslint', 'uglify']

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

rollupConfig.format = 'iife'
rollupConfig.sourceMap = 'inline'

module.exports = function (config) {
  config.set({
    frameworks: ['mocha', 'sinon-chai'],
    browsers: ['PhantomJS'],
    files: [
      '../html5/test/render/vue/**/*.js'
      // '../html5/test/render/vue/components/*.js'
    ],

    exclude: [
      '../html5/test/render/vue/helper.js',
      '../html5/test/render/vue/vender/**/*.js'
      // '../html5/test/render/vue/examples/**/*.js'
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
      '../html5/test/**/*.js': ['rollup', 'coverage'],
      // '../html5/test/**/!(components|examples)/*.js': ['rollup', 'coverage']
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
