<template>
  <div>
    <panel title = 'image-onload' :padding-body='0'>
      <div style='flex-direction:row'>
        <image class='mr-base' style="width: 300;height: 300;" src="https://gw.alicdn.com/tps/TB1bEMYKXXXXXaLaXXXXXXXXXXX-360-388.png" @load="onload"></image>
        <image class='mr-base' style="width: 300;height: 300;border-width:2px;" src="https://cn.vuejs.org/images/logo.png1" @load="onloadFailed"></image>
      </div>
      <panel title='校验结果：'>
        <text test-id='imgSize' style="font-size:30px">{{size}}</text>
        <text test-id='download' style="font-size:30">{{download}}</text>
      </panel>
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
    data : {
      size:"-1,-1",
      download:'success'
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      onload : function(e) {
        nativeLog(JSON.stringify(e))
        this.size = e.size.naturalWidth + ',' + e.size.naturalHeight;
      },
      onloadFailed:function(e) {
        if (e.success){
          this.download = 'success'; 
        }else {
          this.download ='failed';
        }
      }
    }
  }
</script>

<style scoped>
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
  .input{
    width: 500px;
    height:100px;
    border-width: 1px;
  }
</style>