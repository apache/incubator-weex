<template>
  <div>
    <panel title = 'storage-event' :padding-body='0'>
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
  var storage = weex.requireModule("storage");
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
        storage.setItem('key', 'value', ()=>{
          this.resultTxt = 'setItem success';
        });
      },
      getItem:function() {
        storage.getItem('key', (e)=>{
          this.resultTxt = 'getItem:'+e.data;
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