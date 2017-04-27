
<template>
  <div class="wrapper" @click="update" >
    <div class="item" compositing=true>
      <div class="itemWrapper">
        <div class="nameWrapper">
          <text class="itemName">Thomas Carlyle</text>
        </div>
        <image class="itemPhoto" src="https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg"></image>
        <div class="descWrapper">
          <text class="itemDesc">Genius only means hard-working all one\'s life</text>
        </div>
      </div>
    </div>

    <div class="item" compositing=false>
      <div class="itemWrapper">
        <div class="nameWrapper">
          <text class="itemName">Thomas Carlyle</text>
        </div>
        <image class="itemPhoto" src="https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg"></image>
        <div class="descWrapper">
          <text test-id="test-text" class="itemDesc">Genius only means hard-working all one\'s life</text>
        </div>
      </div>
    </div>
  </div>
</template>

<style>
  .wrapper {
    align-items: center; 
    opacity: 0.8;
  }
  .itemWrapper {
    flex:1;
    border-top-right-radius: 80;
    border-bottom-right-radius: 80;
    border-top-width: 10;
    border-top-color: red;
    border-style: dashed;
    align-items: center;
    background-color: white;
  }
  .nameWrapper {
    background-color:rgba(255,0,0,0.5);
    border-radius: 30;
    overflow: visible;
    border-color: black;
    border-width: 10;
  }
  .item {
    margin-top: 120px; 
    background-color: #CCCCCC;
    align-items: center;
  }
  .itemName {
    font-size:28;
    color:#333333;
    line-height:42;
    text-align:left;
    margin-top: 24;
  }
  .itemPhoto {
    margin-top: 18;
    width: 220;
    height: 220;
    margin-bottom: 18;
    border-radius: 30;
    overflow: hidden;
    background-color: blue;
  }
  .itemDesc {
    font-size:24;
    margin:12;
    color:#999999;
    line-height:36;
    text-align:left;
  }
</style>

<script>
  module.exports = {
    data: {
    },
    methods: {
    }
  }
</script>
