var path = require('path')
var webpack = require('webpack')
var pkg = require('../package.json')

var sourceMapPlugin = new webpack.SourceMapDevToolPlugin({
  test: /\.js$/
})

var version = pkg.subversion.browser

var date = new Date().toISOString().split('T')[0].replace(/\-/g, '')
var banner = `(function(s) {console.log(s)})` +
  `('START WEEX HTML5: ${version} Build ${date}');`;

var bannerPlugin = new webpack.BannerPlugin(banner, {
  raw: true
})

module.exports = {
  entry: './html5/browser',
  output: {
    path: './dist',
    filename: 'browser.common.js',
  },
  node: {
    global: false
  },
  module: {
    loaders: [
      {
        test: /\.js$/,
        exclude: /node_modules/,
        loaders: ['babel']
      },
      { test: /\.json$/, loader: 'json'},
      { test: /\.css$/, loader: 'style-loader!css-loader' }
    ]
  },
  plugins: [bannerPlugin, sourceMapPlugin]
}
