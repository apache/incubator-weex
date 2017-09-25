---
title: Set Up Development Environment 
type: guide
order: 1.1
version: 2.1
has_chapter_content: true
---

# Set up development environment 

Using [dotWe](http://dotwe.org/vue) is a good choice, but if you want to develop locally on your own machine, you will need to set up your development environment.

You will need Node.js and the [Weex CLI](https://github.com/weexteam/weex-toolkit).

You can install Node using [nvm](https://github.com/creationix/nvm) (Simple bash script to manage multiple active node.js versions). Run the following commands in a Terminal after installing nvm:

```bash
$ nvm install 6.10.0
$ nvm use 6.10.0
```

Node.js comes with npm, which lets you install the Weex CLI.

**NOTE: ** After the `weex-toolkit`1.0.8 version, the npm-shrinkwrap.json` npm5 specification has been added to lock the package dependencies, so the npm version <5 needs to be updated by` npm i npm @latest -g` npm version, please confirm whether the version is correct before use.

Run the following command in a Terminal:

```bash
$ npm install -g weex-toolkit
$ weex -v
```
You can use 'update@x.x.x' command to update specify version of weex-devtool,weex-previewer,weex-builder and weexpack
```
weex update weex-devtool@latest //latest sign means install latest version
```
**NOTE: ** If you get an error like "permission error", try installing with `sudo`.


Then you can use the Weex command to verify that the installation is successful:

![](https://img.alicdn.com/tfs/TB1NBhdQXXXXXXzXFXXXXXXXXXX-712-343.png)

### Generate a new Weex project

You can use the CLI to generate a Weex project called "awesome-project". Run the following command in a Terminal:

```bash
$ weex create awesome-project
```

Then enter the awesome-project folder, the CLI has been for us to generate a standard project structure.

### Usage

We enter the awesome-project folder and install dependencies with the following commands:

```bash
npm install
```

Then we run `npm run serve` in root directory to start watch mode and static server.

Finally, we can see the Weex page in `http://localhost:8080/index.html`.

You can modify this page in `src/index.vue`. The code is shown below:

```html
<template>
  <div class="wrapper" @click="update">
    <image :src="logoUrl" class="logo"></image>
    <text class="title">Hello {{target}}</text>
    <text class="desc">Now, let's use vue to build your weex app.</text>
  </div>
</template>

<style>
  .wrapper { align-items: center; margin-top: 120px; }
  .title { padding-top:40px; padding-bottom: 40px; font-size: 48px; }
  .logo { width: 360px; height: 156px; }
  .desc { padding-top: 20px; color:#888; font-size: 24px;}
</style>

<script>
  export default {
    data: {
      logoUrl: 'http://img1.vued.vanthink.cn/vued08aa73a9ab65dcbd360ec54659ada97c.png',
      target: 'World'
    },
    methods: {
      update: function (e) {
        this.target = 'Weex'
        console.log('target:', this.target)
      }
    }
  }
</script>
```

If you're curious to learn more about technical details, continue on to the next section. And don't forget to write code at [dotWe](http://dotwe.org) and preview at anytime.
