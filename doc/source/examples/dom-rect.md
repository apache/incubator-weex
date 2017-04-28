---
title: dom-rect
type: example
layout: example
web_bundle_url: ../js/examples/dom-rect.web.js
---

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