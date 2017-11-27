---
title: <scroller>
type: references
order: 2.9
version: 2.1
---

# &lt;scroller&gt;

<span class="weex-version">v0.6.1+</span>

`<scroller>` 是一个竖直的，可以容纳多个排成一列的子组件的滚动器。如果子组件的总高度高于其本身，那么所有的子组件都可滚动。

**注意：** `<scroller>` 可以当作根元素或者嵌套元素使用。此组件的滚动方向是垂直方向的形式。

## 子组件

支持任意类型的 Weex 组件作为其子组件。 其中，还支持以下两个特殊组件作为子组件：

- `<refresh>`

  用于给列表添加下拉刷新的功能。

  使用文档请查看 [`<refresh>`](./refresh.html)

- `<loading>`

  `<loading>` 用法与特性和 `<refresh>` 类似，用于给列表添加上拉加载更多的功能。

  使用文档请查看 [`<loading>`](./loading.html)

## 特性

- `show-scrollbar {boolean}`：可选值为 `true`/ `false`，默认值为 `true`。控制是否出现滚动条。

- `scroll-direction {string}`：可选为 `horizontal` 或者 `vertical`，默认值为 `vertical` 。定义滚动的方向。
  - `scroll-direction`定义了 scroller 的滚动方向，`flex-direction` 定义了 scroller 的布局方向，两个方向必须一致。
  - `scroll-direction` 的默认值是 `vertical`, `flex-direction` 的默认值是 `row`。
  - 当需要一个水平方向的 scroller 时，使用 `scroll-direction:horizontal` 和 `flex-direction: row`。
  - 当需要一个竖直方向的 scroller 时，使用 `scroll-direction:vertical` 和 `flex-direction: column`。由于这两个值均是默认值，当需要一个竖直方向的 scroller 时，这两个值可以不设置。
- `loadmoreoffset {number}`：默认值为 0，触发 `loadmore` 事件所需要的垂直偏移距离（设备屏幕底部与页面底部之间的距离）。当页面的滚动条滚动到足够接近页面底部时将会触发 `loadmore` 这个事件。

  ![mobile_preview](../images/scroller_1.jpg)

- ~~`loadmoreretry {number}`：默认值为 0，当 `loadmore` 失败时是否重置 `loadmore` 相关的 UI，值不一样就会重置。~~ 该属性已废弃，请使用`resetLoadmore()`函数实现重置`loadmore`的操作。
- `offset-accuracy {number}` <sup class="wx-v">0.11+</sup>：控制`onscroll`事件触发的频率，默认值为10，表示两次`onscroll`事件之间列表至少滚动了10px。注意，将该值设置为较小的数值会提高滚动事件采样的精度，但同时也会降低页面的性能。

- `offset-accuracy`：默认值是0，触发 `scroll` 事件所需要的垂直偏移距离。

## 样式

- 通用样式：支持所有通用样式

  - 盒模型
  - `flexbox` 布局
  - `position`
  - `opacity`
  - `background-color`

  查看 [组件通用样式](../common-style.html)

## 事件

- `loadmore` <sup class="wx-v">v0.5+</sup>：如果滚动到底部将会立即触发这个事件，你可以在这个事件的处理函数中加载下一页的列表项。
- `scroll` <sup class="wx-v">0.11+</sup>: 列表发生滚动时将会触发该事件，事件的默认抽样率为10px，即列表每滚动10px触发一次，可通过属性`offset-accuracy`设置抽样率。    参见 [scroll event demo](http://dotwe.org/vue/9ef0e52bacaa20182a693f2187d851aa)。 

  事件中 event 对象属性：
  - `contentSize {Object}`：列表的内容尺寸
    - `width {number}`: 列表内容宽度
    - `height {number}`: 列表内容高度
  - `contentOffset {Object}`: 列表的偏移尺寸
    - `x {number}`: x轴上的偏移量
    - `y {number}`: y轴上的偏移量

- `scrollstart` <sup class="wx-v">v0.17+</sup>：列表刚开始发生滚动的时候会触发该事件，可以在此执行滚动开始的逻辑。

- `scrollend` <sup class="wx-v">v0.17+</sup>：列表结束滚动的时候会触发该事件，可以在此执行滚动结束的逻辑。

- 通用事件

  支持所有通用事件：

  - `click`
  - `longpress`
  - `appear`
  - `disappear`

  查看 [通用事件](../common-event.html)

## 扩展

### scrollToElement(node, options)

滚动到列表某个指定项是常见需求，`<list>` 拓展了该功能支持滚动到指定 `<cell>`。通过 `dom` module 访问，更多信息可参考 [dom module](../modules/dom.html) 。

### resetLoadmore() <sup class="wx-v">0.9+</sup>
在默认情况下，触发`loadmore`事件后，如果列表中内容没有发生变更，则下一次滚动到列表末尾时将不会再次触发`loadmore`事件，你可以通过调用`resetLoadmore()`方法来打破这一限制，调用该方法后，下一次滚动到列表末尾时将强制触发`loadmore`。

#### 参数

- `node {node}`：指定目标节点。
- `options {Object}`：
    - `offset {number}`：一个到其可见位置的偏移距离，默认是0

## 约束

**不允许**相同方向的 `<list>` 或者 `<scroller>` 互相嵌套，换句话说就是嵌套的 `<list>`/`<scroller>` 必须是不同的方向。

举个例子，**不允许**一个垂直方向的 `<list>` 嵌套的一个垂直方向的 `<scroller>` 中，但是一个垂直方向的 `<list>` 是可以嵌套的一个水平方向的 `<list>` 或者 `<scroller>` 中的。

## 示例

```html
<template>
  <div class="wrapper">
    <scroller class="scroller">
      <div class="row" v-for="(name, index) in rows" :ref="'item'+index">
        <text class="text" :ref="'text'+index">{{name}}</text>
      </div>
    </scroller>
    <div class="group">
      <text @click="goto10" class="button">Go to 10</text>
      <text @click="goto20" class="button">Go to 20</text>
    </div>
  </div>
</template>

<script>
  const dom = weex.requireModule('dom')

  export default {
    data () {
      return {
        rows: []
      }
    },
    created () {
      for (let i = 0; i < 30; i++) {
        this.rows.push('row ' + i)
      }
    },
    methods: {
      goto10 (count) {
        const el = this.$refs.item10[0]
        dom.scrollToElement(el, {})
      },
      goto20 (count) {
        const el = this.$refs.item20[0]
        dom.scrollToElement(el, { offset: 0 })
      }
    }
  }
</script>

<style scoped>
  .scroller {
    width: 700px;
    height: 700px;
    border-width: 3px;
    border-style: solid;
    border-color: rgb(162, 217, 192);
    margin-left: 25px;
  }
  .row {
    height: 100px;
    flex-direction: column;
    justify-content: center;
    padding-left: 30px;
    border-bottom-width: 2px;
    border-bottom-style: solid;
    border-bottom-color: #DDDDDD;
  }
  .text {
    font-size: 45px;
    color: #666666;
  }
  .group {
    flex-direction: row;
    justify-content: center;
    margin-top: 60px;
  }
  .button {
    width: 200px;
    padding-top: 20px;
    padding-bottom: 20px;
    font-size: 40px;
    margin-left: 30px;
    margin-right: 30px;
    text-align: center;
    color: #41B883;
    border-width: 2px;
    border-style: solid;
    border-color: rgb(162, 217, 192);
    background-color: rgba(162, 217, 192, 0.2);
  }
</style>
```

[try it](http://dotwe.org/vue/2f22f14fb711d88515e63c3f67bed46a)
