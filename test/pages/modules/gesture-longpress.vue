<template>
  <div>
    <panel title = 'gesture-longpress' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="longpress" @longpress.native="longpress"></button>
      </div>
      <panel title='校验结果：'>
        <text test-id='result' style="font-size:30px">{{resultTxt}}</text>
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
  var clipboard = weex.requireModule("clipboard");
  module.exports = {
    data : {
      resultTxt:'',
      screenX:'',
      screenY:'',
      identifier:'',
      pageX:'',
      pageY:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      longpress:function(e) {
        if(typeof window != 'undefined')
        {
          this.screenY=e.touch.screenY;
          this.identifier=e.touch.identifier;
          this.screenX=e.touch.screenX;
          this.pageY=e.touch.pageY;
          this.pageX=e.touch.pageX;

        }
        else
        {
          var dict= e.changedTouches[0];
          this.screenY=dict.screenY;
          this.identifier=dict.identifier;
          this.screenX=dict.screenX;
          this.pageY=dict.pageY;
          this.pageX=dict.pageX;
          this.resultTxt = JSON.stringify(dict)
        }

      }
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