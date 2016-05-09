#!/usr/bin/env node
'use strict';

let path = require('path');
let fs = require('fs');

let argv = require('yargs')
  .usage('Create component')
  .example('npm run create homepage -o ./example')
  .boolean('p')
  .alias('p', 'parted')
  .describe('p', 'create parted files "js/css/html"')
  .demand('o')
  .alias('o', 'out')
  .describe('o', 'the folder that write the file(s)')
  .boolean('f')
  .alias('f', 'force')
  .describe('f', 'force to replace exsisting file(s)')
  .argv;

function getWeContent(name) {
  return `<template>
  <div class="wxc-${name}"></div>
</template>

<style>
.wxc-${name} {}
</style>

<script>
module.exports = {
  data: {

  },
  methods: {

  }
};
</script>
`;
}

function getPartedContent(name) {
  const JS = `import template from './${name}.html';
import style from './${name}.css';

module.exports = {
  template,
  style,
  data: {

  },
  methods: {

  }
};
`;

  const CSS = `.${name} {}`;
  const HTML = `<div class="wxc-${name}"></div>`;

  return {
    JS,
    CSS,
    HTML
  }
}

const CWD = process.cwd();
const output = path.join(CWD, argv.out);
argv._.forEach(function(name) {
  if (argv.parted) {
    let content = getPartedContent(name);
    fs.writeFileSync(path.join(output, name + '.js'), content.JS, 'utf-8');
    fs.writeFileSync(path.join(output, name + '.css'), content.CSS, 'utf-8');
    fs.writeFileSync(path.join(output, name + '.html'), content.HTML, 'utf-8');
  } else {
    let content = getWeContent(name);
    fs.writeFileSync(path.join(output, name + '.we'), content, 'utf-8');
  }
});