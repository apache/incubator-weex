<template>
  <div>
  <div class="items-box">
    <a weex-type="a" class="items" @click="clickitem('Tap1')">
      <text class="text">Tap1</text>
    </a>
    <a weex-type="a" class="items" @click="clickitem('Tap2')">
      <text class="text">Tap2</text>
    </a>
  </div>
  <div class="items-box">
    <a weex-type="a" class="items" @click="clickitem('Tap in the a tag')" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Tap in the A tag</text>
    </a>
    <a weex-type="a" class="items" @click="preventDefault" :href="href" target="_blank">
      <text class="text">Prevent A tag from href jumping</text>
    </a>
  </div>
  <div class="items-box">
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
  </div>
  <div class="items-box">
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
  </div>
  <div class="items-box">
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
    <a weex-type="a" class="items" href="http://weex.apache.org/cn/references/" target="_blank">
      <text class="text">Url</text>
    </a>
  </div>
  

  <div class="wrapper" @click="backTop" v-if="show">
    <text class="text">Tap</text>
  </div>
  </div>
</template>

<style scoped>
  .items-box{
    flex-direction: row;
    justify-content: space-between;
    -webkit-box-orient: horizontal;
    -webkit-box-direction: normal;
    -webkit-box-pack: justify;
  }
  .items{
    flex-flow: row nowrap; 
    align-items: center; 
    overflow: hidden; 
    position: relative; 
    top: 0px; left: 0px; 
    box-sizing: border-box; 
    width: 374px; 
    height: 564px; 
    flex-basis: auto; 
    font-size: 0px; 
    border-color: rgb(0, 0, 0); 
    border-style: solid; 
    background-color: rgb(245, 245, 245); 
    margin-bottom: 2px; 
    text-align: center; 
  }
  .flexRow {
    flex-direction: row;
  }

  .flexColumn {
    flex-direction: column;
  }

  .text {
    flex: 1;
    text-align: center;
  }
  
  .fixed {
    position: fixed;
    bottom: 50px;
    left: 50px;
    width: 300px;
    height: 50px;
    align-items: center;
    justify-content: center
  }

  .shopImg {
    width: 220px;
    height: 220px;
  }

  .list {
    flex-direction: column;
    overflow: hidden;
    width: 750px;
    height: 100%;
    background-color: #dddddd;
  }

  .cell {
    background-color: #dddddd;
    flex-direction: column;
    width: 750px;
  }

  .test {
    width:60px;
    height:60px;
  }

  .shopDiv {
    flex-direction: column;
    background-color: #ffffff;
    margin: 5px;
    padding: 10px;
    border-width: 1px;
    border-color: #cccccc;
    overflow: visible;
  }

  .shopRowList {
    flex-direction: column;
    border-width: 1px;
    border-color: #cccccc;
    overflow: visible;
    margin: 5px;
    padding: 10px;
    background-color: #ffffff;
  }

  .shopHeader {
    flex-direction: row;
    width: 720px;
  }

  .shopFooter {
    flex-direction: row;
    width: 720px;
  }

  .smallImg {
    width: 20px;
    height: 20px;
  }
  .wrapper {
    background-color: #f8f8f8;
    border:1px solid #ddd;
    position: fixed;
    bottom: 20px;
    right: 20px;
    width: 88px;
    height: 88px;
    z-index: 10100;
    flex-flow: row nowrap; 
    align-items: center; 
    cursor: pointer;
  }
  .backTop {
    width: 88px;
    height: 88px;
  }
</style>

<script>
  var modal = weex.requireModule('modal')
  var dom = weex.requireModule('dom')
  module.exports = {
    data: function () {
      return {
        show: true,
        href: 'http://weex.apache.org/cn/references/'
      }
    },
    methods: {
      clickitem: function(e) {
        modal.toast({
          message:'click' + e
        })
      },
      preventDefault: function(e) {
        this.href = 'javascript:void(0)'
        modal.toast({
          message:'click'
        })
      },
      backTop: function () {
        let self = this
        window.scrollTo(0, 0)
        this.show = false
      }
    }

  }
</script>
