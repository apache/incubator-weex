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
var path = require('path');
var fs = require('fs');
var webpack = require('webpack');
// var bannerWebpackPlugin = require('./examples-banner-plugin')

var entry = {};
var bannerExcludeFiles = [];

function walk(dir) {
  dir = dir || '.'
  var directory = path.join(__dirname, '../examples', dir);
  fs.readdirSync(directory)
    .forEach(function(file) {
      var fullpath = path.join(directory, file);
      var stat = fs.statSync(fullpath);
      var extname = path.extname(fullpath);
      if (stat.isFile() && (extname === '.we' || extname === '.vue')) {
        var name = path.join('examples', 'build', dir, path.basename(file, extname));
        entry[name] = fullpath + '?entry=true';
        if (extname === '.we') {
          bannerExcludeFiles.push(name + '.js')
        }
      } else if (stat.isDirectory() && file !== 'build' && file !== 'include') {
        var subdir = path.join(dir, file);
        walk(subdir);
      }
    });
}

walk();

var banner = '// { "framework": "Vue" }\n'

var bannerPlugin = new webpack.BannerPlugin(banner, {
  raw: true,
  exclude: bannerExcludeFiles
})

module.exports = {
  entry: entry,
  output : {
    path: '.',
    filename: '[name].js'
  },
  module: {
    loaders: [
      {
        test: /\.(we|vue)(\?[^?]+)?$/,
        loader: 'weex'
      }
    ]
  },
  plugins: [bannerPlugin]
}
