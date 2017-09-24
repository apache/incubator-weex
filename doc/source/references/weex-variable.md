---
title: Weex Variable  
type: references
order: 7
version: 2.1
---

# Weex instance variable

Each Weex page has a separate weex variable, which exists in the JS context. They hold a single instance or method of the current Weex page.   

## `weex.config`

This variable contains all the environment information for the current Weex page, including not only:       

+ `BundleUrl`: string: The URL of the JS bundle.
+ `Env: Object`: environment object.
+ `WeexVersion: string`: Weex sdk version.
+ `AppName: string`: application name.
+ `AppVersion: string`: app version.
+ `Platform: string`: platform information, that is iOS, Android or Web.
+ `osName: string`: OS name information, that is iOS, Android.
+ `OsVersion: string`: system version.
+ `DeviceModel: string`: device model (native application only).
+ `DeviceWidth: number`: device width
+ `DeviceHeight: number`: device height.  In Weex, the default width of viewport is 750px, thus you can obtain the height of screen by `height = 750/deviceWidth*deviceHeight` if the width of viewport remains its default value.

`weex.requireModule(module: string): Object`     

Get all the methods of a native module, such as:    

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

Returns the document object of the current Weex page.

## `weex.supports`

<span class="weex-version">v0.15+</span>

`weex.supports` is a method used to detect whether a feature is supported in the current environment.

> Only supported in Weex DSL 2.0 (.vue).

### API

```js
weex.supports(condition : String) : Boolean | Null
```

#### Parameter

+ a formatted string: `@{type}/{name}`.

The `type` must be "component" or "module", the `name` can be tag name, module name or the method name in specific module.

#### Return Value

+ if supported, returns `true`.
+ if unsupported, returns `false`.
+ if unclear, returns `null`.

### Example

```js
// Detects whether the specific component is supported
weex.supports('@component/slider') // true
weex.supports('@component/my-tab') // false

// Detects whether the specific module is supported
weex.supports('@module/stream')  // true
weex.supports('@module/abcdef')  // false

// Detects whether the method in specific module is supported
weex.supports('@module/dom.getComponentRect') // true
weex.supports('@module/navigator.jumpToPage') // false

// invalid input
weex.supports('div') // null
weex.supports('module/*') // null
weex.supports('@stream/fetch') // null
weex.supports('getComponentRect') // null
```
