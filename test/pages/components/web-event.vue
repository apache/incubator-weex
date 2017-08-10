<template>
  <div>
    <panel title = 'web-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <web class="mr-base web" :src = "src"  @pagestart = "pagestartEvt" @pagefinish = "pagefinishEvt" @error = "errorEvt"></web> 
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
  module.exports = {
    data : {
      src:'http://h5.m.taobao.com',
      resultTxt:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      pagefinishEvt:function(e){
        this.resultTxt = 'page finish'
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
  .web{
    width: 500px;
    height:300px;
    border-width: 1px;
  }
</style>