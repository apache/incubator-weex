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
"use strict"

const fs = require('fs')
const path = require('path')
const gzip = require('zlib').createGzip()
const pkg = require('../package.json')
const rollup = require('rollup')
const watch = require('rollup-watch')
const webpack = require('webpack')
const scan = require('weex-vue-bundle-util')
const webExamplesWebpackConfig = require('./webpack.examples.web.config')
const exec = require('child_process').execSync

const getConfig = require('./config')

// create dist folder
if (!fs.existsSync('dist')) {
  fs.mkdirSync('dist')
}

let isWatch = false
if (process.argv[3]) {
  isWatch = process.argv[3] === '--watch' || process.argv[3] === '-w'
}

// build specific package
if (process.argv[2]) {
  build(process.argv[2])
}
else {
  console.log('\nPlease specify the package you want to build. [native, runtime, browser, vue]')
}

function extend(to, ...froms) {
  froms.forEach(function (from) {
    for (const key in from) {
      if (from.hasOwnProperty(key)) {
        to[key] = from[key]
      }
    }
  })
  return to
}

function runRollupOnWatch(config) {
  const watcher = watch(rollup, config)
  watcher.on('event', event => {
    switch ( event.code ) {
      case 'STARTING':
        console.log('checking rollup-watch version...')
        break

      case 'BUILD_START':
        console.log('bundling...')
        break

      case 'BUILD_END':
        console.log('bundled in ' + path.relative(process.cwd(), config.dest)
          + ' (' + event.duration + 'ms)')
        console.log('Watching for changes...' )
        break

      case 'ERROR':
        console.error('ERROR: ', event.error)
        break

      default:
        console.error('unknown event', event)
    }
  })
}

function runRollup (config) {
  return new Promise((resolve, reject) => {
    rollup.rollup(config).then(bundle => {
      bundle.write(config).then(() => {
        report(config.dest)
        resolve()
      })
    })
  })
}

function getAllEntries (rootDir) {
  return fs.readdirSync(rootDir)
    .filter(function (file) {
      return fs.statSync(path.join(rootDir, file)).isDirectory()
    })
    .map(function (file) {
      var fullpath = path.join(rootDir, file)
      var entry = path.join(fullpath, 'src/index.js')
      var dest = path.join(fullpath, 'dist/index.js')
      var name = file.replace(/-(\w)/g, function ($0, $1) {
        return $1.toUpperCase()
      })
      return { entry, dest, name }
    })
}

function absolutePath (p) {
  return path.join(__dirname, p)
}

function buildForWebExamples (config, minifyConfig) {
  config.entry = minifyConfig.entry = absolutePath('../entry.js')
  config.dest = absolutePath('../dist/render.vue.js')
  delete config.banner
  config.plugins = config.plugins.filter(function (plugin) {
    return plugin.name !== 'eslint'
  })
  minifyConfig.dest = absolutePath('../dist/render.vue.min.js')
  delete minifyConfig.banner

  // bundle web examples.
  scan(webpack, webExamplesWebpackConfig)
    .then(function (res) {
      var pkgs = res.pkgs
      var names = []
      var str = pkgs.map(function (pkgName) {
        var name = pkgName
          .replace('weex-vue-', '')
          .replace(/-(\w)/g, function ($0, $1) {
            return $1.toUpperCase()
          })
          + 'Mod'
        names.push(name)
        try {
          var version = require(`${pkgName}/package.json`).version
        } catch (err) {
          exec(`npm install ${pkgName}`)
        }
        return `import ${name} from '${pkgName}'\n`
      }).join('')
      str += `export default [\n${names.join(',  \n')}\n]\n`
      return fs.writeFileSync(absolutePath('../weex-vue-plugins.js'), str)
    })
    .then(function () {
      console.log(`\n => start to build weex-vue-render for examples.\n`)
      return new Promise((resolve, reject) => {
        runRollup(config).then(() => {
          runRollup(minifyConfig).then(() => {
            zip(minifyConfig.dest, resolve)
          })
        })
      })
    })
}

function build (name) {
  let pkgName = 'weex-js-framework'
  switch (name) {
    case 'native': pkgName = 'weex-js-framework'; break;
    case 'runtime': pkgName = 'weex-js-runtime'; break;
    case 'browser': pkgName = 'weex-web-render'; break;
    case 'vue': pkgName = 'weex-vue-render'; break;
    case 'vue-plugins': pkgName = 'weex-vue-render-plugins'; break;
    case 'vue-core': pkgName = 'weex-vue-render-core'; break;
    case 'web-examples': pkgName = 'weex-vue-render-core'; break;
  }

  const config = getConfig(pkgName)
  const minifyConfig = getConfig(pkgName, true)

  if (name === 'web-examples') {
    buildForWebExamples(config, minifyConfig)
  }
  else if (pkgName === 'weex-vue-render-plugins') {
    // build multiple packages in a loop.
    console.log(`\n => start to build ${name} (${pkgName})\n`)
    const entries = getAllEntries(path.join(__dirname, '../packages/weex-vue-plugins'))
    entries.forEach(function (item) {
      const info = {
        moduleName: item.name,
        entry: item.entry
      }
      const itemConfig = extend({}, config, info, { dest: item.dest })
      const itemMinConfig = extend({}, minifyConfig, info, { dest: item.dest.replace(/\.js$/, '.min.js') })
      runRollup(itemConfig)
        .then(() => {
          return runRollup(itemMinConfig)
        })
    })
  }
  else if (isWatch) {
    return runRollupOnWatch(config)
  }
  else {
    console.log(`\n => start to build ${name} (${pkgName})\n`)
    return new Promise((resolve, reject) => {
      runRollup(config).then(() => {
        runRollup(minifyConfig).then(() => {
          zip(minifyConfig.dest, resolve)
        })
      })
    })
  }
}

function zip (filePath, callback) {
  const read = fs.createReadStream(filePath)
  const write = fs.createWriteStream(filePath + '.gz')
  read.pipe(gzip).pipe(write).on('close', () => {
    report(filePath + '.gz')
    callback && callback()
  })
}

function now () {
  const time = Date.now() - (new Date()).getTimezoneOffset() * 60000
  return (new Date(time)).toISOString().replace('T', ' ').substring(0, 16)
}

function report (filePath) {
  const size = (fs.statSync(filePath).size / 1024).toFixed(2) + 'KB'
  const file = path.relative(process.cwd(), filePath)
  console.log(` => write ${file} (${size})`)
}
