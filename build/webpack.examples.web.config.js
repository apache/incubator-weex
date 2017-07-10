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
var fs = require('fs-extra');
var webpack = require('webpack');

var entry = {};
var bannerExcludeFiles = [];
var webSrcDirectory = path.join(__dirname, '../examples/web-entry');

function getEntryFileContent (entryPath, vueFilePath) {
  const relativePath = path.relative(path.join(entryPath, '../'), vueFilePath);
  return 'var App = require(\'' + relativePath + '\')\n'
    + 'App.el = \'#root\'\n'
    + 'new Vue(App)\n'
}

function walk(dir) {
  dir = dir || '.';
  var directory = path.join(__dirname, '../examples', dir);
  var entryDirectory = path.join(webSrcDirectory, dir);
  fs.readdirSync(directory)
    .forEach(function(file) {
      var fullpath = path.join(directory, file);
      var stat = fs.statSync(fullpath);
      var extname = path.extname(fullpath);
      if (stat.isFile() && extname === '.vue') {
        var entryFile = path.join(entryDirectory, path.basename(file, extname) + '.js');
        fs.outputFileSync(entryFile, getEntryFileContent(entryFile, fullpath));
        var name = path.join('examples', 'build/vue-web', /*path.relative('vue', dir)*/dir, path.basename(file, extname));
        entry[name] = entryFile + '?entry=true';
      } else if (stat.isDirectory() && file !== 'build' && file !== 'include') {
        var subdir = path.join(dir, file);
        walk(subdir);
      }
    });
}

walk();

var banner = '// NOTE: for vue2.0 and platform:web only.\n'

var bannerPlugin = new webpack.BannerPlugin(banner, {
  raw: true,
  exclude: bannerExcludeFiles
})

module.exports = {
  entry: entry,
  output: {
    path: '.',
    filename: '[name].js'
  },
  module: {
    loaders: [
      {
        test: /\.js$/,
        loaders: ['babel-loader'],
        exclude: /node_modules/
      }, {
        test: /\.vue(\?[^?]+)?$/,
        loaders: ['vue-loader']
      }
    ]
  },
  vue: {
    optimizeSSR: false,
    /**
     * important! should use postTransformNode to add $processStyle for
     * inline style prefixing.
     */
    compilerModules: [
      {
        postTransformNode: el => {
          el.staticStyle = `$processStyle(${el.staticStyle})`
          el.styleBinding = `$processStyle(${el.styleBinding})`
        }
      }
    ],
  },
  plugins: [bannerPlugin]
}
