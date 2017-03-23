# Vue Web Renderer for Weex

This is a group of Vue 2.x components for Weex, which have the same behavior with the Weex built-in components on the native.

It can be run on browser only based on Vue 2.x (without Weex).

## use vue-loader

**NOTE: ** you should use `vue-loader` to bundle your `.vue` file, not using `weex-loader`.

you should add config for `vue-loader` as below (`vue-loader` version should be `>= 11.2.0`)

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
  autoprefixer: {
    browsers: ['last 7 versions']
  }
},
```
