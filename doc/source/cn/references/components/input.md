---
title: <input>
type: references
order: 2.5
version: 2.1
---

# &lt;input&gt;

Weex 内置的 `<input>` 组件用来创建接收用户输入字符的输入组件。 `<input>` 组件的工作方式因 `type` 属性的值而异，比如 `<text>`， `password`，`url`，`email`，`tel` 等。

**注意：** 

此组件不支持 `click` 事件。请监听 `<input>` 或 `change` 来代替 `click` 事件。

## 子组件

不支持子组件。

## 特性

- `type {string}`：控件的类型，默认值是 `<text>`。`type` 值可以是 `text`，`password`，`url`，`email`，`tel` ，`number` 。每个 `type` 值都符合 W3C 标准。
- `value {string}`：组件的接收到的输入字符。
- `placeholder {string}`：提示用户可以输入什么。 提示文本不能有回车或换行。
- `disabled {boolean}`：布尔类型的数据，表示是否支持输入。通常 `click` 事件在 `disabled` 控件上是失效的。
- `autofocus {boolean}`：布尔类型的数据，表示是否在页面加载时控件自动获得输入焦点。
- `maxlength {nubmer}`：<sup class="wx-v">v0.7</sup>一个数值类型的值，表示输入的最大长度。
- `return-key-type {string}`：<sup class="wx-v">v0.11</sup>键盘返回键的类型,支持 defalut;go;next;search;send,done。

## 样式

- `placeholder-color {color}`：placeholder 字符颜色。默认值是 `#999999`


- 伪类<span class="api-version">v0.9.5+</span>: `input` 支持以下伪类：

  * `active`
  * `focus`
  * `disabled`
  * `enabled`

- text styles
  - 支持 `color`
  - 支持 `font-size`
  - 支持 `font-style`
  - 支持 `font-weight`
  - 支持 `text-align`

  查看 [文本样式](../text-style.html)

- 通用样式：支持所有通用样式
  - 盒模型
  - `flexbox` 布局
  - `position`
  - `opacity`
  - `background-color`

  查看 [组件通用样式](../common-style.html)

## 事件

- `input`: 输入字符的值更改。

  事件中 event 对象属性：

  - `value`: 触发事件的组件；
  - `timestamp`: 事件发生时的时间戳,仅支持Android。

- `change`: 当用户输入完成时触发。通常在 `blur` 事件之后。

  事件中 event 对象属性：

  - `value`: 触发事件的组件；

  - `timestamp`: 事件发生时的时间戳,仅支持Android。

- `focus`: 组件获得输入焦点。

  事件中 event 对象属性：

  - `timestamp`: 事件发生时的时间戳,仅支持Android。

- `blur`: 组件失去输入焦点。

  事件中 event 对象属性：

  - `timestamp`: 事件发生时的时间戳,仅支持Android。
- `return`: 键盘点击返回键。

    事件中 event 对象属性：

    - `returnKeyType`: 事件发生时的返回键类型。
    - `value`: 触发事件的组件的文本；

- 通用事件

  **注意：**
  不支持 `click` 事件。 请监听 `input` 或 `change` 事件代替。

  支持以下通用事件：

  - `longpress`
  - `appear`
  - `disappear`

  查看 [通用事件](../common-event.html)



## Methods

- `focus()` <span class="api-version">v0.9+</span>

  `focus()` 方法用于将 `input` 组件聚焦。

- `blur()` <span class="api-version">v0.9+</span>

  `blur()` 方法用于从 `input` 组件中移除焦点并关闭软键盘（如果它具有焦点）。
- `setSelectionRange(selectionStart,selectionEnd)`  <span class="api-version">v0.11+</span>设置文本选区
  - `selectionStart {number}`：设置文本选区的起始点
  - `selectionEnd {number}`：设置文本选区的起终点
- `getEditSelectionRange(callback[selectionStart,selectionEnd])`  <span class="api-version">v0.11+</span>设置文本选区
    - `selectionStart {number}`：获取文本选区的起始点
    - `selectionEnd {number}`：获取文本选区的起终点
## 约束

目前不支持 `this.$el(id).value = ''` 这种方式改写 input value。只支持在 `<input>` 组件的 `input`、`change` 事件中改写。

## 示例

```html
<template>
  <div>
    <div>
      <text style="font-size: 40px">oninput: {{txtInput}}</text>
      <text style="font-size: 40px">onchange: {{txtChange}}</text>
      <text style="font-size: 40px">onreturntype: {{txtReturnType}}</text>
      <text style="font-size: 40px">selection: {{txtSelection}}</text>

    </div>
    <scroller>
      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = text</text>
        </div>
        <input type="text" placeholder="Input Text" class="input" :autofocus=true value="" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = password</text>
        </div>
        <input type="password" placeholder="Input Password" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = url</text>
        </div>
        <input type="url" placeholder="Input URL" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = email</text>
        </div>
        <input type="email" placeholder="Input Email" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = tel</text>
        </div>
        <input type="tel" placeholder="Input Tel" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = time</text>
        </div>
        <input type="time" placeholder="Input Time" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = number</text>
        </div>
        <input type="number" placeholder="Input number" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = date</text>
        </div>
        <input type="date" placeholder="Input Date" class="input" @change="onchange" @input="oninput" max="2017-12-12" min="2015-01-01"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = default</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="default" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = go</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="go" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = next</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="next" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = search</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="search" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = send</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="send" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = done</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="done" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>


      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">function focus() & blur()</text>
        </div>
        <div style="flex-direction: row;margin-bottom: 16px;justify-content: space-between">
          <text class="button" value="Focus" type="primary" @click="focus"></text>
          <text class="button" value="Blur" type="primary" @click="blur"></text>
        </div>

        <input type="text" placeholder="Input1" class="input" value="" ref="input1"/>
      </div>


      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input selection</text>
        </div>
        <div style="flex-direction: row;margin-bottom: 16px;justify-content: space-between">
          <text class="button" value="setRange" type="primary" @click="setRange"></text>
          <text class="button" value="getSelectionRange" type="primary" @click="getSelectionRange"></text>
        </div>
        <input type="text"  ref="inputselection" placeholder="please input" value="123456789"  class="input" @change="onchange" @return = "onreturn" @input="oninput"/>
      </div>



    </scroller>
  </div>
</template>

<style scoped>
  .input {
    font-size: 60px;
    height: 80px;
    width: 750px;
  }
  .button {
    font-size: 36;
    width: 200;
    color: #41B883;
    text-align: center;
    padding-top: 10;
    padding-bottom: 10;
    border-width: 2;
    border-style: solid;
    margin-right: 20;
    border-color: rgb(162, 217, 192);
    background-color: rgba(162, 217, 192, 0.2);
  }
</style>

<script>
  module.exports = {
    data: function () {
      return {
        txtInput: '',
        txtChange: '',
        txtReturnType: '',
        txtSelection:'',
        autofocus: false
      };
    },
    methods: {
      ready: function () {
        var self = this;
        setTimeout(function () {
          self.autofocus = true;
        }, 1000);
      },
      onchange: function (event) {
        this.txtChange = event.value;
        console.log('onchange', event.value);
      },
      onreturn: function (event) {
        this.txtReturnType = event.returnKeyType;
        console.log('onreturn', event.type);
      },
      oninput: function (event) {
        this.txtInput = event.value;
        console.log('oninput', event.value);
      },
      focus: function () {
        this.$refs['input1'].focus();
      },
      blur: function () {
        this.$refs['input1'].blur();
      },
      setRange: function() {
        console.log(this.$refs["inputselection"]);
        this.$refs["inputselection"].setSelectionRange(2, 6);
      },
      getSelectionRange: function() {
        console.log(this.$refs["inputselection"]);
        var self = this;
        this.$refs["inputselection"].getSelectionRange(function(e) {
          self.txtSelection = e.selectionStart +'-' + e.selectionEnd;
        });
      }
    }
  };
</script>
```

[体验一下](http://dotwe.org/vue/3470e4d0194f3879a72d38e2ab02cc9f)
