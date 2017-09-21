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

Run the following command in a Terminal:

```bash
$ npm install -g weex-toolkit
$ weex -v

v1.0.3
weex-builder : v0.2.4
weex-previewer : v1.3.4
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

Then we run `npm run dev & npm run serve` in root directory to start watch mode and static server.

Finally, we can see the Weex page in `http://localhost:8080/index.html`.

You can modify this page in `src/foo.vue`. The code is shown below:

```html
<template>
  <div class="wrapper">
    <text class="weex">Hello Weex!</text>
    <text class="vue">Hello Vue!</text>
  </div>
</template>

<style scoped>
  .wrapper {
    flex-direction: column;
    justify-content: center;
  }
  .weex {
   font-size: 60px;
   text-align: center;
   color: #1B90F7;
  }
  .vue {
   font-size: 60px;
   text-align: center;
   margin-top: 30px;
   color: #41B883;
  }
</style>
```

If you're curious to learn more about technical details, continue on to the next section. And don't forget to write code at [dotWe](http://dotwe.org) and preview at anytime.
