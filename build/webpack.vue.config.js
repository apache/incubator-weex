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
var fs = require('fs')
var path = require('path')
var webpack = require('webpack')

var bannerPlugin = new webpack.BannerPlugin('//banner\n', {
  raw: true
})

var devtool = '#inline-source-map'
var filename = 'vue-renderer.js'

var plugins = [
  bannerPlugin,
  new webpack.DefinePlugin({
    'process.env': {
      NODE_ENV: JSON.stringify(process.env.NODE_ENV)
    }
  }),
  new webpack.optimize.OccurenceOrderPlugin()
]

if (process.env.NODE_ENV === 'production') {
  devtool = null
  filename = 'vue-renderer.min.js'
  plugins.push(new webpack.optimize.UglifyJsPlugin({
    compress: {
      warnings: false
    }
  }))
}

module.exports = {
  entry: path.resolve('./html5/render/vue/index.js'),
  output: {
    path: path.resolve('./dist'),
    filename: filename
  },
  module: {
    loaders: [
      {
        test: /\.js$/,
        loader: 'babel',
        exclude: /node_modules|vender/
      }, {
        test: /\.vue$/,
        loader: 'vue'
      }
    ]
  },
  devtool: devtool,
  plugins: plugins
}
