---
title: Get Started  
type: guide
order: 1
version: 2.1
has_chapter_content: true
---

# Get Started

Weex is a framework for building Mobile cross-platform high performance UI applications. Developers can leverage their existing knowledge of VueJS (a powerful JavaScript view library), by writing `*.vue` files to build native applications or pages. This page will help you to write a **Weex** application in 2 minutes.

## Introduction to VueJS

>[VueJS](https://vuejs.org/) (pronounced /vjuː/, like view) is a progressive framework for building user interfaces. Unlike other monolithic frameworks, Vue is designed from the ground up to be incrementally adoptable. The core library is focused on the view layer only, and is very easy to pick up and integrate with other libraries or existing projects. On the other hand, Vue is also perfectly capable of powering sophisticated Single-Page Applications when used in combination with modern tooling and supporting libraries.

**VueJS is developed by [Evan You](https://twitter.com/youyuxi).**

Weex now fully supports VueJS 2.x officailly. Weex put Vue 2.x as its built-in JS Framework, making it powerful for building native apps!.

## Hello world Example

The easiest way to try Weex is to use the [Playground App](../playground.html) to write a [Hello World](http://dotwe.org/vue/4d5a0471ece3daabd4681bc6d703c4c1) example at [dotWe](http://dotwe.org). No installation is required and you do not even have to write native code. 

To test Weex online using our Playground App:

- Install the [Playground App](../playground.html) on your phone.
- Open [the Hello World example](http://dotwe.org/vue/4d5a0471ece3daabd4681bc6d703c4c1) in a new tab, click run, and then use the Playground App to scan the QR code.

Nailed it! 

If you took the procedure above, you can see simple HTML semantic tags, CSS styles and Javascript code. This is one of the simplest Weex examples. The demo applications renders a "Hello World" in the page. Please note that this is not a Web page. You are running native!.

![mobile_preview](https://img.alicdn.com/tps/TB1Ymw3OpXXXXcvXpXXXXXXXXXX-500-1013.jpg)

### What happened?

As shown in the following code：

```html
<template>
  <div>
    <text class="text">{{text}}</text>
  </div>
</template>

<style>
  .text {
    font-size: 50;
  }
</style>

<script>
  export default {
    data () {
      return {
        text: 'Hello World.'
      }
    }
  }
</script>
```

It's too easy，right? Take a look at the syntax, that is vue.

You can try to modify the Hello World example if you have previous knowledge of building VueJS examples, then generate a new QR code to scan. If you don't have previous knowledge of VueJS, don't worry, this guide will teach you just that. Moreover, you can always learn from the [VueJS Guide](https://vuejs.org/v2/guide).
