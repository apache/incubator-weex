<template>
  <div>
    <panel title = 'modal-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="alertClick" @click.native="alertClick"></button>
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
  var GET_URL = 'http://httpbin.org/get';
  var modal = weex.requireModule("modal");
  module.exports = {
    data : {
      resultTxt:'',
      sendtype:'json',
      detail:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      alertClick:function() {
        var params = {
          'message':'message',
          'okTitle':'OK',
        }
        modal.alert(params,()=>{
          this.resultTxt = 'alert success';
        })
      },
      
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