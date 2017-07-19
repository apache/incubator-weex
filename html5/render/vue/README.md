# Vue Web Renderer for Weex

This is a group of Vue 2.x components for Weex, which have the same behavior with the Weex built-in components on the native.

It can be run on browser only based on Vue 2.x (without Weex).

## how to use

If you prefer npm way, you should manually call `init` function to pass Vue constructor to weex.

```javascript
import Vue from 'vue'
import weex from 'weex-vue-render'
weex.init(Vue)

import App from 'App.vue'
new Vue(App)
```

If you perfer cdn way, and use script tag to import a script link, just import it into your html file, it will auto-init and you can just ignore it.

```html
<script>{{Vue}}</script>
<script>{{weex-vue-render}}</script>
<script>{{your js bundle}}</script>
```

## use vue-loader to bundle .vue file

NOTE: you should inject `$processStyle` to preprocess vnode's style in vue-loader's postTransformNode hook.

```javascript
vue: {
  /**
    * important! should use postTransformNode to add $processStyle for
    * inline style prefixing.
    */
  compilerModules: [
    {
      postTransformNode: el => {
        el.staticStyle = `$processStyle(${el.staticStyle})`
        el.styleBinding = `$processStyle(${el.styleBinding})`
      }
    }
  ],
}
```

## component -> dom map

| component | dom element | children | note |
| --- | --- | ---- | --- |
| scroller | /main | /main/div/x | - |
| list | /main | /main/div/x | - |
| cell | /sect | /sect/x | - |
| header | /head | /head/x | - |
| loading | /asid | /aside/x | - |
| loading-indicator | /mark | /mark/x | - |
| refresh | /asid | /aside/x | - |
| slider | /nav | x -> /ul/li/x, indicator -> /nav/nav | - |
| indicator | /nav | - | indicator points: /nav/mark |
| a | /a | /a/x | - |
| div | /div | /div/x | - |
| image | /figure | - | - |
| input | /input | - | - |
| switch | /span | - | round button: /span/small |
| text | /p | - | - |
| textarea | /textarea | - | - |
| video | /video | - | - |
| web | /iframe | - | - |
