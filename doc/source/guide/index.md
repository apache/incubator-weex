---
title: Get Started  
type: guide
order: 1
version: 2.1
has_chapter_content: true
---

# Get Started

Weex is a framework for building Mobile cross-platform high performance UI. Developers can write `*.vue` files to build native pages or an App. This page will help you write a Weex page in 2 minutes.

## What is Vue?

[Vue.js](https://vuejs.org/) is an excellent progressive JavaScript framework written by [Evan You](https://twitter.com/youyuxi). Now Weex and Vue has supported each other officially. Weex put Vue 2.x as its built-in JS Framework, and Vue has already been able to develop native mobile app.

## Hello world

The easiest way to try Weex is to use the [Playground App](../playground.html) and write a Hello World example at [dotWe](https://dotwe.org). You don't need to consider installing the development environment or writing native code, just do the following two things:

- Install the [Playground App](../playground.html) for your phone.
- Open [the Hello World example](http://dotwe.org/vue/4d5a0471ece3daabd4681bc6d703c4c1) in a new tab, click run, and then use the Playground App to scan the QR code.

Nailed it! In this case, We can see HTML semantic tags, CSS styles and Javascript code. This is one of the simplest Weex examples. It rendered a "Hello World" in the page. Please note that this is not a Web page.

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

It's too easy，right? Let's focus on the syntax, obviously this is vue.

You can try to modify the Hello World, then generate a new QR code to scan.
