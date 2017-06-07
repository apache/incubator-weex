# Vue Web Renderer for Weex

This is a group of Vue 2.x components for Weex, which have the same behavior with the Weex built-in components on the native.

It can be run on browser only based on Vue 2.x (without Weex).

## use vue-loader

**NOTE: ** after `v0.11.3` there's no need to add `$processStyle` and `autoprefixer` in your vue-loader config anymore. The runtime render will take care of it once for all.

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
