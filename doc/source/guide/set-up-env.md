---
title: Setup Development Environment
type: guide
order: 1.1
version: 2.1
has_chapter_content: true
---

# Setup Development Environment

Using [dotWe](http://dotwe.org/vue) is a good choice, but if you want to develop locally on your own machine, you will need to set up your development environment.

You will need Node.js and the [Weex CLI](https://github.com/weexteam/weex-toolkit).

Install Node.js using [nvm](https://github.com/creationix/nvm) (Simple bash script to manage multiple active Node.js versions). Run the following commands in a terminal after installing nvm:

```bash
$ nvm install 6.10.0
$ nvm use 6.10.0
```

And `npm` comes with Node.js, with witch you can install the Weex command line tools.

**NOTE: ** After upgrading the `weex-toolkit` to v1.0.8, the `npm-shrinkwrap.json` npm 5 specification has been added to lock the package dependencies, it is needed to upgrade your npm version to 5 above by command: `npm install npm @latest -g` if your version is lower than this, please check your npm version before using it.

Run the following commands in a terminal:

```bash
$ npm install -g weex-toolkit
$ weex -v
```
You can use 'weex update <component>@x.x.x' to update weex-devtool, weex-previewer, weex-builder and weexpack to a specific version.
```
weex update weex-devtool@latest // Here latest means to install the latest version
```

**NOTE: ** If you receive an error like "permission error", try another time with `sudo`.

Then you can use the Weex command to verify if the installation is successful:

![](https://img.alicdn.com/tfs/TB1NBhdQXXXXXXzXFXXXXXXXXXX-712-343.png)

### Generate a new Weex project

Use the command line tool to generate a Weex project called "awesome-project". Run the following command in a terminal:

```bash
$ weex create awesome-project
```

Then enter the awesome-project folder, you will see a standard project skeleton has been generated.

### Usage

In the awesome-project folder, install dependencies with the following command:

```bash
npm install
```

Then run `npm run serve` in root directory to start watching mode and static server.

Finally, you can access the Weex page in `http://localhost:8080/index.html`.

This page can be modified in `src/index.vue`. The codes are shown below:

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

For more technical details, please continue to read the next section. And don't forget to write and preview your codes at [dotWe](http://dotwe.org/vue).
