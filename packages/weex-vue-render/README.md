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

## use render-core and plugins

You can juse use a **render-core** in your page, and define your needs for the plugins (components and API modules) in your entry file, so that your would save a lot of bundle size and bandwidth. On the other hand, The full version contains all the built-in components and API modules, which will waste a lot of bundle size and bandwidth if you haven't use most of them.

To use the render-core and the plugins, just install the plugins in your entry file.

```javascript
import Vue from 'vue'

// import render-core.
import weex from 'weex-vue-render/dist/index.core'

// import the plugins (components and modules) you want to use.
import slider from 'weex-vue-slider'
import stream from 'weex-vue-stream'

// init weex.
weex.init(Vue)

// install the plugins.
weex.install(slider)
weex.install(stream)

import App from 'App.vue'
new Vue(App)
```

#### built-in components in render-core

These components have already integrated into render-core. You don't have to import and install them:

* div
* text
* image
* scrollable components (scroller, list, waterfall)

#### available built-in components for you to install

| npm module name | component name |
| --- | --- |
| weex-vue-a | a |
| weex-vue-input | input |
| weex-vue-slider | slider / slider-neighbor |
| weex-vue-switch | switch |
| weex-vue-textarea | textarea |
| weex-vue-video | video |
| weex-vue-web | web |

#### available API modules for you to install

| npm module name | API module name |
| --- | --- |
| weex-vue-animation | animation |
| weex-vue-clipboard | clipboard |
| weex-vue-dom | dom |
| weex-vue-event | event |
| weex-vue-geolocation | geolocation |
| weex-vue-global-event | globalEvent |
| weex-vue-modal | modal |
| weex-vue-nav | navigator |
| weex-vue-storage | storage |
| weex-vue-stream | stream |
| weex-vue-websocket | websocket |
| weex-vue-webview | webview |

Some of the above plugins may not be mentioned in weex documents. Just ignore them if you have no idear about what they are and how to use them.

You can develop more plugins for your app. Accordingly the same plugin for other two platforms (iOS and Android) should also be built and intergrated in your app if you want it run on all three platforms.

#### scan your code

Use `weex-vue-bundle-util` to scan you code, which will return a promise with a object to tell which components and modules you have used in your project, so that can generate a entry file accordingly to import those plugins.

https://github.com/weexteam/weex-vue-bundle-util

You can see the examples of weex built in this way by running the script `npm run build:vue:examples`, and use the `<script src="./dist/render.vue.js"></script>` as the render in the `vue.html` file.

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

## changelog

#### 0.12.1

* pack a core sdk for weex-vue-render into `weex-vue-render/dist/index.core.js`.
* pack modules and components into npm packages, and make them able to install.

#### 0.12.3

* support component method `image.save` for `<image>` component.
* distinguish gesture events with the same named events generated elsewhere.
* fix fast click event.
* fix `modal.toast` on android.
* fix css prefixer for gradients.

#### 0.12.4

* fix scanning style in production with uglified code.

#### 0.12.8

* optimize event binding.
* fix a-jump interception condition.

#### 0.12.9

* event handling ignore none-vue element.

#### 0.12.10

* fix accessing vm in dom which is not a vue component.

#### 0.12.11

* fix styles in firefox.
* implement methods about selection range.

#### 0.12.12

* fix NS (svg) bug in vue.js@v2.4.3

#### 0.12.13

* reset `env.deviceHeight` after window's resizing.

#### 0.12.14

* fix loading & refresh: pulling down wrongly triggered.

#### 0.12.16

* not to prevent default behaviour of click events unless the click-binding element is inside a `<a>` link, or it is a `<a>` link and has a `prevent` attribute on it.

#### 0.12.18

* support offset appear.
* fix lazyload failing when frequently updating images' src. Typical scenario is skeleton replacing.

#### 0.12.20

* fix scrollToElement on latest chrome.

#### 0.12.21

* fix `stream.fetch` for adding wrong suffix to the request url.

#### 0.12.22

* fix image lazyloading.

#### 0.12.24

* add try catch to accessing localStorage.
* support image sprite.

#### 0.12.25

* fix indicator position in one page slider.

#### 0.12.26

* fix unit wx, which is a straight px value not to adapt to screens.

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
