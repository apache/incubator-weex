<template>
  <scroller style='alignItems:center'>
    <image style="width: 500;height: 500;" src="https://gw.alicdn.com/tps/TB1bEMYKXXXXXaLaXXXXXXXXXXX-360-388.png" @load="onload"></image>
    <text test-id='imgSize' style="font-size:30">{{size}}</text>
    <image style="width: 300;height: 300;border-width:2px;" src="https://cn.vuejs.org/images/logo.png1" @load="onloadFailed"></image>
    <text test-id='download' style="font-size:30">{{download}}</text>
  </scroller>
</template>
<script>
  module.exports = {
    data : function(){
      return {
      size:"-1,-1",
      download:'success'
      }
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