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
'use strict'

const ConcatSource = require('webpack/lib/ConcatSource')

const isProd = process.env.NODE_ENV === 'production'
const dateFormat = require('dateformat')
const appVersion = require('../package.json').version
const now = new Date()
const packTime = dateFormat(now, 'yyyymmddHHMMss')
const date = dateFormat(now, 'yyyymmdd')
const childProcess = require('child_process')
const hash = childProcess.execSync('git rev-parse --short HEAD').toString().trim()
const brash = childProcess.execSync('git rev-parse --abbrev-ref HEAD').toString().trim()

const crypto = require('crypto')
function md5 (text) {
  return crypto.createHash('md5').update(text, 'utf8').digest('hex')
}

function generateBanner (digest) {
  const tmp = ` * {
 *   version: "${appVersion}.${date}${isProd ? '-release' : ''}",
 *   create: "${packTime}",
 *   git: "${brash}--${hash}",
 *   digest: "${digest}"
 * }
 !*/
`
  const length = tmp.length
  return `/*!${length}
${tmp}`
}

function BannerWebpackPlugin (options) {
  this.options = options || {}
}

BannerWebpackPlugin.prototype.apply = function (compiler) {
  compiler.plugin('emit', (compilation, callback) => {
    // const chunkKey = Object.keys(this.chunks)
    compilation.chunks.map((chunk) => {
      const distChunk = chunk.files[0]

      const source = compilation.assets[distChunk].source()
      compilation.assets[distChunk].source = () => {
        return source
      }
      compilation.assets[distChunk] = new ConcatSource(generateBanner(md5(source)), compilation.assets[distChunk])
    })
    callback()
  })
}

BannerWebpackPlugin.prototype.findAsset = function (compilation, chunk) {
  const chunks = compilation.chunks
  for (let i = 0, len = chunks.length; i < len; i++) {
    if (chunks[i].name === chunk) {
      return chunks[i].files[0]
    }
  }

  return null
}

module.exports = BannerWebpackPlugin
