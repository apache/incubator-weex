---
title: Weex 实例变量
type: references
order: 7
version: 2.1
has_chapter_content: true
---

# Weex 实例变量

每个 Weex 页面的 JS 上下文中都有一个相互独立的 `weex` 变量，用来持有当前 Weex 页面相关的单例内容或方法。

## `weex.config`

该变量包含了当前 Weex 页面的所有环境信息，包括不仅限于：

* `bundleUrl: string`: JS bundle 的 URL，和页面的 URL 一致。
* `env: Object`: 环境对象。
    * `weexVersion: string`: Weex sdk 版本。
    * `appName: string`: 应用名字。
    * `appVersion: string`: 应用版本。
    * `platform: string`: 平台信息，是 iOS、Android 还是 Web。
    * `osName: string`: iOS或者android，表示操作系统的名称.
    * `osVersion: string`: 系统版本。
    * `deviceModel: string`: 设备型号 (仅原生应用)。
    * `deviceWidth: number`: 设备宽度。Weex 默认以宽度为 750px 做适配渲染，要获得750px下的屏幕高度，可以通过 `height = 750/deviceWidth*deviceHeight`  公式获得，可以使用到 CSS 中，用来设置全屏尺寸
    * `deviceHeight: number`: 设备高度。

## `weex.requireModule(module: string): Object`

获取某个 native module 的所有方法，比如：

```html
<template>
  <div><text>Hello World</text></div>
</template>
<script>
  var modal = weex.requireModule('modal')
  modal.toast({
    message: 'I am a toast.',
    duration: 3
  })
</script>
```

## `weex.document: Document`

返回当前 Weex 页面的文档对象。

## `weex.supports`

<span class="weex-version">v0.15+</span>

`weex.supports` 可以用来检测当前环境中是否支持某个特性。

> 目前仅在 Weex DSL 2.0 (.vue) 中支持。

### API

```js
weex.supports(condition : String) : Boolean | Null
```

#### 参数

+ 特定格式的字符串：`@{type}/{name}`。

`type` 必须是 "component" 或者 "module"。`name` 可以是标签名、模块名，也可以指定模块中的某个方法名（和模块名用 `.` 隔开）。

#### 返回值

+ 支持，则返回 `true`。
+ 不支持，则返回 `false`。
+ 参数格式错误或无法确定是否支持，则返回 `null`。

### 例子

```js
// 检测是否支持某个组件
weex.supports('@component/slider') // true
weex.supports('@component/my-tab') // false

// 检测是否支持某个模块
weex.supports('@module/stream')  // true
weex.supports('@module/abcdef')  // false

// 检测是否支持模块中的某个方法
weex.supports('@module/dom.getComponentRect') // true
weex.supports('@module/navigator.jumpToPage') // false

// 无效的输入
weex.supports('div') // null
weex.supports('module/*') // null
weex.supports('@stream/fetch') // null
weex.supports('getComponentRect') // null
```
