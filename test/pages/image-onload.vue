<template>
  <scroller>
    <image style="width: 500;height: 500;" src="https://gw.alicdn.com/tps/TB1bEMYKXXXXXaLaXXXXXXXXXXX-360-388.png" @load="onload"></image>
    <text test-id='imgSize' style="font-size:30">{{size}}</text>
  </scroller>
</template>
<script>
  module.exports = {
    data : function(){
      return {
      size:"-1,-1"
      }
    },
    methods : {
      onload : function(e) {
        nativeLog(JSON.stringify(e))
        this.size = e.size.naturalWidth + ',' + e.size.naturalHeight;
      }
    }
  }
</script>