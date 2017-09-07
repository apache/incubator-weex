<template>
  <div>
    <panel title = 'viewport-basic' :padding-body='0'>
      <panel title='screenWidth：'>
        <text test-id='device-width' style="font-size:30px">{{_deviceWidth}}</text>
      </panel>
      <panel title='screenHeight：'>
        <text test-id='device-height' style="font-size:30px">{{_deviceHeight}}</text>
      </panel>
      <div test-id='test-point' style='width:320;height:160;background-color:red;'>
        <text>宽度测试</text>
      </div>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 

测试方式：
  * 
  * 
  * 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  const meta = weex.requireModule('meta')
  // 配置 viewport 的宽度为 640px
  meta.setViewport({  
    width: 640
  })

  module.exports = {
    data : {
      _deviceWidth:'',
      _deviceHeight:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
    },
    created:function(){
      this._deviceWidth = weex.config.env.deviceWidth;
      this._deviceHeight = weex.config.env.deviceHeight;
    }
  }
</script>

<style scoped>
  .origin{background-color: #FFFFDF;width: 200;height: 50;}
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>