---
title: 搭建开发环境 
type: guide
order: 1.1
version: 2.1
has_chapter_content: true
---

# 搭建开发环境

使用 [dotWe](http://dotwe.org/vue) 对 Weex 尝鲜是一个不错的选择，但如果你想更专业的开发 Weex， dotWe 就不怎么够用了。本节会教你如何搭建本地开发环境进行 Weex 开发。

## 第一步：安装依赖

Weex 官方提供了 weex-toolkit 的脚手架工具来辅助开发和调试。首先，你需要 Node.js 和 [Weex CLi](https://github.com/weexteam/weex-toolkit)。

安装 Node.js 方式多种多样，最简单的方式是在 [Node.js 官网](https://nodejs.org/en/) 下载可执行程序直接安装即可。

对于 Mac，可以使用 [Homebrew](http://brew.sh/) 进行安装：

```bash
brew install node
```

> 更多安装方式可参考 [Node.js 官方信息](https://nodejs.org/en/download/)

安装完成后，可以使用以下命令检测是否安装成功：

```bash
$ node -v
v6.11.3
$ npm -v
3.10.10
```

通常，安装了 Node.js 环境，npm 包管理工具也随之安装了。因此，直接使用 npm 来安装 weex-toolkit。

> npm 是一个 JavaScript 包管理工具，它可以让开发者轻松共享和重用代码。Weex 很多依赖来自社区，同样，Weex 也将很多工具发布到社区方便开发者使用。

**注意: ** 在`weex-toolkit`1.0.8版本后添加了npm5规范的`npm-shrinkwrap.json`用于锁定包依赖，故npm版本<5的用户需要通过`npm i npm@latest -g`更新一下npm的版本，使用前请确认版本是否正确。

```bash
$ npm install -g weex-toolkit
$ weex -v //查看当前weex版本
```

weex-toolkit也支持直接升级子依赖，如：
```
weex update weex-devtool@latest //@后标注版本后，latest表示最新
```

国内开发者可以考虑使用淘宝的 npm 镜像 —— [cnpm](https://npm.taobao.org/) 安装 weex-toolkit


```bash
$ npm install -g cnpm --registry=https://registry.npm.taobao.org
$ cnpm install -g weex-toolkit
```

*提示：*

如果提示权限错误（*permission error*），使用 `sudo` 关键字进行安装

```bash
$ sudo cnpm install -g weex-toolkit
```

安装结束后你可以直接使用 `weex` 命令验证是否安装成功，它会显示 `weex` 命令行工具各参数：

![](https://img.alicdn.com/tfs/TB1NBhdQXXXXXXzXFXXXXXXXXXX-712-343.png)

## 第二步：初始化

然后初始化 Weex 项目：

```bash
$ weex create awesome-project
```

执行完命令后，在 `awesome-project` 目录中就创建了一个使用 Weex 和 Vue 的模板项目。

## 第三步：开发

之后我们进入项目所在路径，weex-toolkit 已经为我们生成了标准项目结构。

在 `package.json` 中，已经配置好了几个常用的 npm script，分别是：

- `build`: 源码打包，生成 JS Bundle
- `dev`: webpack watch 模式，方便开发
- `serve`: 开启HotReload服务器，代码改动的将会实时同步到网页中

我们先通过 `npm install` 安装项目依赖。之后运行根目录下的 `npm run dev & npm run serve` 开启  watch 模式和静态服务器。

然后我们打开浏览器，进入 `http://localhost:8080/index.html` 即可看到 weex h5 页面。 

初始化时已经为我们创建了基本的示例，我们可以在 `src/index.vue` 中查看。

代码如下所示：

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

关于 Weex 语法部分，你可以直接参考 [Vue Guide](https://vuejs.org/v2/guide/)，这里不再重复介绍。如果您想了解有关技术详情的更多信息，请继续阅读下一节。并且不要忘记在 [dotWe](http://dotwe.org/vue) 写代码并随时预览。
