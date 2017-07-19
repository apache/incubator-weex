---
title: Event Bubble 
type: references
order: 1.3
version: 2.1
---

# Event Bubble <span class="api-version">v0.13+</span>

Weex 2.0 implements the W3C standard event bubbling mechanism.

### Usage

```html
<template>
  <div class="root" @click="rootClick" bubble="true">
    <text style="font-size: 40px;">{{rootText}}</text>
    <div class="outer" @click="parentClick">
      <div>
        <text style="font-size: 40px;">{{parentText}}</text>
      </div>
      <text class="inner" @click="click">{{innerText}}</text>
    </div>
  </div>
</template>

<script>
  module.exports = {
    data: {
      innerText: 'click me',
      parentText: '',
      rootText: ''
    },
    methods: {
      click: function(e) {
        this.innerText = 'inner bubble'
      },
      parentClick: function(e) {
        this.parentText = 'parent bubble'
      },
      rootClick: function(e) {
        this.rootText = 'root bubble'
      }
    }
  }
</script>

<style scoped>
  .inner {
    font-size: 40px;
    text-align: center;
    background-color: #7a9b1b;
    padding: 40px;
  }

  .outer {
    font-size: 40px;
    text-align: center;
    background-color: #9b7a1b;
    padding: 120px;
  }

  .root {
    font-size: 40px;
    text-align: center;
    background-color: #7a1b9b;
    padding: 80px;
  }
</style>
```

[try it](http://dotwe.org/vue/fa2957ce3e9eb47ad9ae1da22d845e95)

Run the above code, open with the client, click on the middle of the elements, you can see the event spread up, followed by the trigger.

### Notice

One thing should be noticed: **For compatibility with previous versions, Weex does not turn on event bubbling by default. You need to add `bubble = "true"` on the root element's properties to turn on the bubbling mechanism. Otherwise, the event will not be propagated upwards, keeping the same effect as the previous version.**

### stopPropagation 

In the event handler function, you can use the `e.stopPropagation()` method to prevent the event from escalating. Note that `e.stopPropagation()` differs from `bubble = "true"`, which affects only the current elements and the propagation of parent elements, without affecting the propagation of child elements; the latter is a switching mechanism that is added for compatibility, Will be a global shutdown or open the bubble mechanism, the two can co-exist, as follows:

```html
<template>
  <div class="root" @click="rootClick" bubble="true">
    <text style="font-size: 40px;">{{rootText}}</text>
    <div class="outer" @click="parentClick">
      <div>
        <text style="font-size: 40px;">{{parentText}}</text>
      </div>
      <text class="inner" @click="click">{{innerText}}</text>
    </div>
  </div>
</template>

<script>
  module.exports = {
    data: {
      innerText: 'click me',
      parentText: '',
      rootText: ''
    },
    methods: {
      click: function(e) {
        this.innerText = 'inner bubble'
      },
      parentClick: function(e) {
        this.parentText = 'parent bubble'
        e.stopPropagation()
      },
      rootClick: function(e) {
        this.rootText = 'root bubble'
        // e.stopPropagation()
      }
    }
  }
</script>

<style scoped>
  .inner {
    font-size: 40px;
    text-align: center;
    background-color: #7a9b1b;
    padding: 40px;
  }

  .outer {
    font-size: 40px;
    text-align: center;
    background-color: #9b7a1b;
    padding: 120px;
  }

  .root {
    font-size: 40px;
    text-align: center;
    background-color: #7a1b9b;
    padding: 80px;
  }
</style>
```

[try it](http://dotwe.org/vue/2cc80e19c9b2430fb780234628065a69)

Run the above code, open with the client, click on the middle of the element, you can see the event up to the parent element is terminated, no longer continue to spread to the root element.
