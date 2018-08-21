<template>
  <div>
    <panel title = 'picker-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="showPicker" @click.native="showPicker"></button>
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
  var picker = weex.requireModule("picker");
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
      showPicker:function() {
        var params =  {
            'value':'2018-01-31',
            'max':'2020-01-31',
            'min':'2010-01-31',
        }
        picker.pickDate(params,  (ret) => {
          if(ret.result == 'success') {
              this.resultTxt = ret.data;
          }
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