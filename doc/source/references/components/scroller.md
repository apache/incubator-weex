---
title: <scroller>
type: references
order: 2.8
version: 2.1
---

# &lt;scroller&gt;

<span class="weex-version">v0.6.1+</span>

A scroller is a component in vertical direction which can have multiple child components in one column. If total height of its child components exceed the height of the scroller, the whole child components will be scrollable.

Notes: A <scroller> can be used as a root element or a embed element. The scroll direction of this component is column, and it can't be changed.


## Child Components

Scroller supports all kinds of components, such as div, text, etc.
And there are two special components that can only be used inside scroller component.

* refresh 0.6.1 used inside list to add pull-down-to-refresh functionality. 
* loading 0.6.1 used inside list to add pull-up-to-load-more functionality. 


## Attributes

* show-scrollbar: true/false whether show the scroll bar or not, default value is true
* scroll-direction: <string> define scroll direction of component, horizontal or vertical
  * `scroll-direction` defines the scrollable axis of scroller and `flex-direction` defines the layout axis of scroller. `scroll-direction` and `flex-direction` must be set to the same direction, otherwise, undefined behavior may happen.
  * Default value for `scroll-direction` is vertical, and for `flex-direction` is row .
  * Use `scroll-direction:horizontal` and `flex-direction: row` when a horizontal layout and scrollable scroller is expected.
  * Use `scroll-direction:vertical` and `flex-direction: column` when a vertical layout and scrollable scroller is expected. But thoes two values are default, if you don't set them, it also works fine.
* loadmoreoffset : <number> default value is 0. The loadmore event will be triggered when the list is loadmoreoffset left to reach the bottom of the list view. e.g. a list has total content length of 1000, and the loadmoreoffset is set to 400, the loadmore event will be triggered when 600 has beed scrolled and there is less than 400 left.
* loadmoreretry : <number> default value 0，whether to reset loadmore related UI when loadmore failed, will be deprecated in further release.
* offset-accuracy：<number> default value is 0, the vertical offset distance required to trigger the scroll event.


## Styles

common styles: check out [common styles for components](../common-style.html)

* support flexbox related styles
* support box model related styles
* support position related styles
* support opacity, background-color etc.


## Events

- `loadmore`  used with loadmoreoffset attribute. if the view has less than loadmoreoffset to scroll down, the onloadmore event will be triggered.
- `scroll`  <sup class="wx-v">0.12+</sup> used with offset-accuracy attribute. This event is fired when the list scrolls. The current contentOffset value is given in this event callback.  See details in [scroll event demo](http://dotwe.org/vue/9ef0e52bacaa20182a693f2187d851aa).
  - `contentSize {Object}`：The content size of the list
    - `width {number}`: List content width
    - `height {number}`: List content height
  - `contentOffset {Object}`: Offset size of the list
    - `x {number}`: Offset on x-axis
    - `y {number}`: Offset on y-axis
- `scrollstart` <sup class="wx-v">v0.17+</sup>：This event is triggered when the list starts to scroll, where you can start the logic of the start of the scrolling.
- `scrollend` <sup class="wx-v">v0.17+</sup>：This event is triggered when the list has finished scrolling, where you can execute the logic that the scroll ends.
- common events: check out the [common events](../common-event.html)
  - `click`
  - `longpress`
  - `appear`
  - `disappear`

## event.html)


## Restrictions

Nested lists or scrollers within the same direction are not supported. In other words. nested lists/scroller must have different directions.
For example, a vertical list nested in a vertical list or scroller is not allowed. However, a vertical list nested in a horizontal list or scroller is legal.

## example

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