---
title: 快速上手
type: guide
order: 1
version: 2.1
has_chapter_content: true
---

# 快速上手

Weex 是一套简单易用的跨平台开发方案，能以 web 的开发体验构建高性能、可扩展的 native 应用，为了做到这些，Weex 与  Vue 合作，使用 Vue 作为上层框架，并遵循 W3C 标准实现了统一的 JSEngine 和 DOM API，这样一来，你甚至可以使用其他框架驱动 Weex，打造三端一致的 native 应用。

## VueJS简介

> [VueJS](https://vuejs.org/) 是 Evan You 开发的渐进式 JavaScript 框架。开发者能够通过撰写 `*.vue` 文件，基于 `<template>`, `<style>`, `<script>` 快速构建组件化的 web 应用。

**VueJS 由 [Evan You](https://twitter.com/youyuxi) 开发维护.**

## Hello World

尝试 Weex 最简单的方法是使用 [Playground App](https://alibaba.github.io/weex/download.html) 和在 [dotWe](http://dotwe.org) 编写一个 [Hello World](http://dotwe.org/vue/4d5a0471ece3daabd4681bc6d703c4c1) 例子。你不需要考虑安装开发环境或编写 native 代码，只需要做下面两件事：

- 为你的手机安装 [Playground App](https://alibaba.github.io/weex/download.html)，当然，Weex 是跨平台的框架，你依然可以使用浏览器进行预览，只是这样你就无法感受到 native 优秀的体验了。
- 在新标签页中打开 [Hello World](http://dotwe.org/vue/4d5a0471ece3daabd4681bc6d703c4c1) 例子，点击预览，然后用  Playground 扫码即可。

在这个例子中，我们看到了熟悉的 HTML 语义化标签、CSS 样式和 Javascript 代码。这是一个最简单的 Weex 示例，它在页面中渲染了一个 “Hello World”。请注意，这不是一个 H5 页面，而是 native 的。

![mobile_preview](https://img.alicdn.com/tps/TB1Ymw3OpXXXXcvXpXXXXXXXXXX-500-1013.jpg)

### 发生了什么？

就如示例代码所示：

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
它太容易了，对吧?看一下语法，这就是vue。

你可以试着修改Hello World示例如果你有以前的知识构建VueJS的例子，然后生成一个新的二维码扫描。如果你没有以前的VueJS知识,别担心,这个指南教你。此外，你可以学习(VueJS指南)(https://vuejs.org/v2/guide)。
