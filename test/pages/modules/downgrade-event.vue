<template>
  <div>
    <panel title = 'downgrade-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="setItem" @click.native="setItem"></button>
        <button class='mr-base' type="info" size="middle" value="getItem" @click.native="getItem"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
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
  import Downgrade from '@weex-project/downgrade'

  Downgrade.condition({
    ios: {
        osVersion: '>1.0',
        appVersion: '>1.0.0',
        weexVersion: '>1',
        deviceModel: ['iPhone5,1']
    },
    android: {
        osVersion: '>1.0',
        appVersion: '>1.0.0',
        weexVersion: '>1',
        deviceModel: ['G-2PW2100']
    }
  });

  var clipboard = weex.requireModule("clipboard");
  module.exports = {
    data : {
      resultTxt:'',
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      setItem:function() {
        clipboard.setString('clipboard-value');
      },
      getItem:function() {
        clipboard.getString((e)=>{
          this.resultTxt = e.data;
        });
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