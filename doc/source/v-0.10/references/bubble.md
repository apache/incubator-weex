---
title: Event Bubble 
type: references
order: 1.3
version: 0.10
---

# Event Bubble <span class="api-version">v0.13+</span>

Weex 1.0 implements the W3C standard event bubbling mechanism.

### Usage

```html
<template>
  <div class="root" onclick="rootClick" bubble="true">
    <div>
      <text style="font-size: 40px;">{{rootText}}</text>
    </div>
    <div class="outer" onclick="parentClick">
      <div>
        <text style="font-size: 40px;">{{parentText}}</text>
      </div>
      <text class="inner" onclick="click">{{innerText}}</text>
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

<style>
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

[try it](http://dotwe.org/weex/dbfeb926e003986e2eea88c8ccdadb92)

Run the above code, open with the client, click on the middle of the elements, you can see the event spread up, followed by the trigger.

### Notice

One thing should be noticed: **For compatibility with previous versions, Weex does not turn on event bubbling by default. You need to add `bubble = "true"` on the root element's properties to turn on the bubbling mechanism. Otherwise, the event will not be propagated upwards, keeping the same effect as the previous version.**

### stopPropagation 

In the event handler function, you can use the `e.stopPropagation()` method to prevent the event from escalating. Note that `e.stopPropagation()` differs from `bubble = "true"`, which affects only the current elements and the propagation of parent elements, without affecting the propagation of child elements; the latter is a switching mechanism that is added for compatibility, Will be a global shutdown or open the bubble mechanism, the two can co-exist, as follows:

```html
<template>
  <div class="root" onclick="rootClick" bubble="true">
    <div>
      <text style="font-size: 40px;">{{rootText}}</text>
    </div>
    <div class="outer" onclick="parentClick">
      <div>
        <text style="font-size: 40px;">{{parentText}}</text>
      </div>
      <text class="inner" onclick="click">{{innerText}}</text>
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

<style>
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

[try it](http://dotwe.org/weex/0cab45a7c62e8bebedd2ffd83a8e1256)

Run the above code, open with the client, click on the middle of the element, you can see the event up to the parent element is terminated, no longer continue to spread to the root element.
