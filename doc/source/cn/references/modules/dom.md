---
title: dom
type: references
order: 3.3
version: 2.1
---

# dom

包含如下可以更新 dom 树的 dom API。

这部分API是通过把 virtual-dom 的消息发送到 native 渲染器来做到的。

开发者在日常开发中，唯一可在 `.vue` 文件中使用的是 `scrollToElement`。
~~你也可以调用 `$scrollTo` 方法来使用它~~

这个页面提及的其他的 API，只在 `callNative` 进程中的 native 渲染器用。
（关于 `callNative` 进程的进一步介绍，可以在 [How it works](../../advanced/how-it-works.html)中的 JS Framework 部分看到 ）

## API
### scrollToElement(node, options)

让页面滚动到那个对应的节点，这个API只能在 `<scroller>` 和 `<list>` 组件中用。

~~这个API也能通过调用VM的方法 `$scrollTo` 来使用（已弃用）~~

要在你的 `.vue` 文件中使用这个 API，可以使用 `weex.requireModule('dom').scrollToElement`。

#### 参数
- `node {Node}`：你要滚动到的那个节点
- `options {Object}`：如下选项
- `offset {number}`：一个到其可见位置的偏移距离，默认是 `0`
- `animated {boolean}` <sup class="wx-v">0.10+</sup>：是否需要附带滚动动画，默认是`true`

#### 示例

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
    /*justify-content: space-around;*/
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

[try it](http://dotwe.org/vue/56e0d256cbb26facd958dbd6424f42b2)

### getComponentRect(ref, callback) <span class="api-version">v0.9.4+</span>

通过标签的 `ref` 获得其布局信息，返回的信息在 `callBack` 中，格式参考如下：

```json
{
  result: true,
  size: {
    bottom: 60,
    height: 15,
    left: 0,
    right: 353,
    top: 45,
    width: 353
  }
}
```

如果想要获取到 Weex 容器的布局信息，可以指定 `ref='viewport'`，调用例子如下：

```html
<template>
  <div class="wrapper" style='margin-top:200px'>
    <div ref="box"  class="box">
      <text class="info">Width: {{size.width}}</text>
      <text class="info">Height: {{size.height}}</text>
      <text class="info">Top: {{size.top}}</text>
      <text class="info">Bottom: {{size.bottom}}</text>
      <text class="info">Left: {{size.left}}</text>
      <text class="info">Right: {{size.right}}</text>
    </div>
    
    <text class="info btn"  @click='click()'>{{this.tip}}</text>
      
  </div>
</template> 

<script>
  const dom = weex.requireModule('dom')
  
 function round(size) {
      var roundSize = {
        'width': Math.round(size.width),
        'height': Math.round(size.height),
        'top': Math.round(size.top),
        'bottom': Math.round(size.bottom),
        'left': Math.round(size.left),
        'right': Math.round(size.right)
      }
      return roundSize
  }

  export default {
    data () {
      return {
        size: {
          width: 0,
          height: 0,
          top: 0,
          bottom: 0,
          left: 0,
          right: 0
        },
        ref:"viewport",
        tip:"get box rect"
      }
    },
    mounted () {
      const result = dom.getComponentRect(this.ref, option => {
        console.log('getComponentRect:', option)
        this.size = round.call(this,option.size);
      })
    },
    
    methods:{
      click:function() {
        if (this.ref === 'viewport') {
          this.ref = this.$refs.box;
          this.tip = "get viewport rect"
        } else {
          this.ref = 'viewport'
          this.tip = "get box rect"
        }
          
         const result = dom.getComponentRect(this.ref, option => {
          console.log('getComponentRect:', option)
          this.size = round.call(this,option.size);
        })
      }
    }
    
  }
</script>

<style scoped>
  .btn {
    margin-top:20px;
    border-width:2px;
    border-style: solid;
    border-radius:10px;
    width:300px;
    margin-left:170px;
    padding-left:35px;
    border-color: rgb(162, 217, 192);
    
  }
  .btn:active {
    background-color: #8fbc8f;
		border-color: gray;
  }
  
  .box {
    align-items:center;
    margin-left: 150px;
    width: 350px;
    height: 400px;
    background-color: #DDD;
    border-width: 2px;
    border-style: solid;
    border-color: rgb(162, 217, 192);
    background-color: rgba(162, 217, 192, 0.2);
  }
  .info {
    font-size: 40px;
    top:30px;
    margin-left:20px;
    font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace;
    color: #41B883;
  }
</style>
```

[try it](http://dotwe.org/vue/d069a9bf0f0781b914f12a9a7b9a1447)


### addRule
`支持版本:v0.12.0`

addRule是可以为dom 添加一条规则，目前支持自定义字体fontFace规则，构建自定义的font-family，可以在[text](../components/text.html#iconfont)使用

#### fontFace

```html
var domModule = weex.requireModule('dom');
domModule.addRule('fontFace', {
    'fontFamily': "iconfont2",
    'src': "url('http://at.alicdn.com/t/font_1469606063_76593.ttf')"
});

```

[try it](http://dotwe.org/vue/95b2c6716f37066d5f44c5c75c979394)

## 其他

dom 还有一些底层接口用于创建 Weex 实例时调用，比如 `createBody`、`updateAttrs` 等，但并未开放供外部使用。
