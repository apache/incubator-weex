---
title: <input>
type: references
order: 2.4
version: 2.1
---

# input

The weex builtin component `input` is used to create input controls to receive the user's input characters. How a `input` component works varies considerably depending on the value of its `type` attribute, such as `text`, `password`, `url`, `email`, `tel` etc.

**Notes:** does not support the common-event `click`. Please listen to the `input` or `change` event instead.

## Child Components

This component supports no child components.

## Attributes

* `type`: the type of controls to display. The default value is `text`, if this attribute is not specified. Possible values are `text`, `password`, `tel`, `email`, `url` , `number` etc. each of which has the same meaning with W3C standard.

* `value`: the value(text) of the control.

* `placeholder`: a hint to the user of which can be entered to the control. The placeholder text must have no carriage returns or line-feeds.

* `disabled`: a boolean attribute indicates that the form control is not available for interaction. In particular, the click event will not be dispatched on disabled controls.

* `autofocus`: a boolean attribute lets you specify that a form control should have input focus when the page loads.

* `maxlength`: <span class="api-version">v0.7+</span> a number value to specify maxlength of input.

* `return-key-type {string}`：<sup class="wx-v">v0.11</sup>the keybord returen key type support defalut;go;next;search;send,done.

## Styles

* placeholder-color: the color of placeholder. Default value is '#999999'.

* Pseudo-class<span class="api-version">v0.9.5+</span>: `input` component support the following pseudo-classes:

  * `active`
  * `focus`
  * `disabled`
  * `enabled`

* text styles: checkout [text styles](../text-style.html)

  * support 'color' style.
  * support 'font-size' style.
  * support 'font-style' style.
  * support 'font-weight' style.
  * support 'text-align' style.

### common styles
check out [common styles for components](../common-style.html)

* support flexbox related styles.
* support box model related styles.
* support 'position' related styles.
* support 'opacity', 'background-color' etc.

## Events

* input: the value of an input character changes.
* change: the change event is fired when a change to the component's value is commited by the user. It always come after a 'blur' event.
* focus: a component has received focus.
* blur: a component has lost focus.
* `return`:the return key click.

     the object property of event:

    - `returnKeyType`: return key Type  of component
    - `value`: component text value

### common events
check out [common events](../common-event.html)

* support 'appear' / 'disappear' event.


### Methoads

 - `focus()` <span class="api-version">v0.9+</span>

  The `focus()` method is used to give focus to an input component and tigger soft keybord(if it can be focused).

 - `blur()`<span class="api-version">v0.9+</span>

  The `blur()` method is used to remove focus from an input component and close soft keybord(if it has focus).

- `setSelectionRange(selectionStart,selectionEnd)`  <span class="api-version">v0.11+</span>set text selection range of input or textarea

  - `selectionStart {number}`:set starting location text selection range
  - `selectionEnd {number}`:set end location text selection range

- `getEditSelectionRange(callback[selectionStart,selectionEnd])`  <span class="api-version">v0.11+</span>get text selection range of input or textarea
    - `selectionStart {number}`:get starting location text selection range
    - `selectionEnd {number}`: get end location text selection range

### Notes
does not support the common-event 'click'. Please listen to the 'input' or 'change' event instead.

### Parameters of events' object

* for 'input' and 'change' events:'value': the value of the component who dispatched this event.'timestamp': the time stamp of the event.
* for 'focus' and 'blur' events:'timestamp': the time stamp of the event.

## Example

```html
<template>
  <div>
    <div>
      <text style="font-size: 40px">oninput: {{txtInput}}</text>
      <text style="font-size: 40px">onchange: {{txtChange}}</text>
      <text style="font-size: 40px">onreturntype: {{txtReturnType}}</text>
    </div>
    <scroller>
      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = text</text>
        </div>
        <input type="text" placeholder="Input Text" class="input" :autofocus=true value="" @change="onchange" @input="oninput" @focus="onfocus" @blur="onblur"/>
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
  const modal = weex.requireModule('modal')
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
      onfocus () {
        console.log('onfocus:');
        modal.toast({
          message: 'onfocus',
          duration: 0.8
        })
      },
      onblur () {
        console.log('onblur:');
        modal.toast({
          message: 'input blur',
          duration: 0.8
        })
      }
    }
  };
</script>
```

[try it](http://dotwe.org/vue/dd83d941d2364f2849e45dc3c5d91ab4)
