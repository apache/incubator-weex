const fs = require('fs')
const path = require('path')
const gzip = require('zlib').createGzip()
const pkg = require('../package.json')
const { rollup } = require('rollup')
const json = require('rollup-plugin-json')
const eslint = require('rollup-plugin-eslint')
const replace = require('rollup-plugin-replace')
const postcss = require('rollup-plugin-postcss')
const nodeResolve = require('rollup-plugin-node-resolve')
const uglify = require('rollup-plugin-uglify')
const commonjs = require('rollup-plugin-commonjs')
const buble = require('rollup-plugin-buble')

// create dist folder
if (!fs.existsSync('dist')) {
  fs.mkdirSync('dist')
}

// build specific package
if (process.argv[2]) {
  switch (process.argv[2]) {
    case 'native': buildNative(); break;
    case 'runtime': buildRuntime(); break;
    default: console.log('\n  invalid package name.\n')
  }
}

// build all packages by default
else {
  buildNative()
  buildRuntime()
}

function runRollup (config) {
  return rollup(config).then(bundle => {
    bundle.write(config)
      .then(() => {
        const size = getSize(config.dest)
        console.log(`write file at: ${config.dest} ${size}`)

        // gzip minified file
        if (/\.min\.js$/.test(config.dest)) {
          const read = fs.createReadStream(config.dest)
          const write = fs.createWriteStream(config.dest + '.gz')
          read.pipe(gzip).pipe(write).on('close', () => {
            const gzipSize = getSize(config.dest + '.gz')
            console.log(`write file at: ${config.dest}.gz ${gzipSize}`)
          })
        }
      })
  })
}

function build (name, options, plugins) {
  const env = options.env
  plugins = plugins || options.plugins
  delete options.env

  // build development version
  if (!env || env === 'development') {
    // console.log(`start building ${name}.js (development version)`)
    runRollup(Object.assign({}, options, {
      dest: `./dist/${name}.js`,
      sourceMap: 'inline',
      plugins: [
        ...plugins,
        replace({ 'process.env.NODE_ENV': JSON.stringify('development') })
      ]
    }))
  }

  // build production version
  if (!env || env === 'production') {
    // console.log(`start building ${name}.min.js (production version)`)
    runRollup(Object.assign({}, options, {
      dest: `./dist/${name}.min.js`,
      plugins: [
        ...plugins,
        replace({ 'process.env.NODE_ENV': JSON.stringify('production') }),
        uglify()
      ]
    }))
  }
}

function buildNative () {
  const version = pkg.subversion.framework
  const banner = `\
  (this.nativeLog || function(s) {console.log(s)})('START JS FRAMEWORK: ${version} Build ${now()}');
  var global = this, process = { env: {}};
  `

  build('native', {
    entry: './html5/render/native/index.js',
    banner,
    format: 'umd',
    plugins: [
      json(),
      eslint({
        exclude: './package.json'
      }),
      nodeResolve({
        jsnext: true,
        main: true
      }),
      commonjs(),
      buble()
    ]
  })
}

function buildRuntime () {
  build('runtime', {
    moduleName: 'weexRuntime',
    entry: './html5/runtime/index.js',
    format: 'umd',
    plugins: [
      eslint(),
      nodeResolve({
        jsnext: true,
        main: true
      }),
      commonjs(),
      buble()
    ]
  })
}

function getSize (file) {
  return (fs.statSync(file).size / 1024).toFixed(2) + 'KB'
}

function now () {
  return new Date().toISOString().replace('T', ' ').substring(0, 16)
}
