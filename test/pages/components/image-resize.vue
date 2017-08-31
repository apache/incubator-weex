<template>
  <div>
    <panel title = 'image-resize' :padding-body='0'>
    	<text value='width>height'></text>
    	<div style="flex-direction: row;">
	      <div class="cell">
	        <image class="wh200x100 border margin" :src='imgSrc' quality='original' :placeholder='imgSrc' ></image>
	      </div>
	      <div class="cell">
	        <image class="wh200x100 border margin" :resize="cover" :src='imgSrc' ></image>
	      </div>
	      <div class="cell">
	        <image class="wh200x100 border margin" :resize="contain" :src='imgSrc'></image>
	      </div>
      </div>
      <text value='width=height'></text>
      <div style="flex-direction: row;">
        <div class="cell">
          <image class="wh200x200 border margin" :src='imgSrc'></image>
        </div>
        <div class="cell">
          <image class="wh200x200 border margin" resize="cover" :src='imgSrc' ></image>
        </div>
        <div class="cell">
          <image class="wh200x200 border margin" resize="contain" :src='imgSrc'></image>
        </div>
      </div>
      <text value='width<height'></text>
      <div style="flex-direction: row;">
        <div class="cell">
          <image class="wh200x400 border margin"  :src='imgSrc'></image>
        </div>
        <div class="cell">
          <image class="wh200x400 border margin" resize="cover"  :src='imgSrc' ></image>
        </div>
        <div class="cell">
          <image class="wh200x400 border margin" resize="contain"  :src='imgSrc'></image>
        </div>
      </div>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 

测试方式：
  * 
  * 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data :function(){
    	return {
	    	imgSrc:"http://img.alicdn.com/tps/TB1zBLaPXXXXXXeXXXXXXXXXXXX-121-59.svg",
	    	cover:'stretch',
	    	contain:'stretch',
	    }
    } ,
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
      h3: require('../include/h3.vue'),
    },
    methods : {
    },
    created:function(){
    	setTimeout(()=>{
    		this.cover='cover';
    		this.contain = 'contain';
    		this.imgSrc='http://gw.alicdn.com/tps/i2/TB1DpsmMpXXXXabaXXX20ySQVXX-512-512.png_200x200.jpg';
    	},0);
    }
  }
</script>

<style scoped>
  .mr-base{
    margin: 10px;
  }
  .wh200x100{width:200;height:100}
  .wh200x200{width:200;height:200}
  .wh200x400{width:200;height:300}
  .cell{flex:1;background-color: #FFFAF0;}
  .border{border-style: solid;border-color: #FF6666;border-width: 2;}
  .margin{margin-left: 5;margin-top: 5;margin-bottom: 5}
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>



