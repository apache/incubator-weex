<template>
  <div>
    <panel title = 'switch-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <switch test-id="switch-obj" class="origin mr-base" @change="change"></switch>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * switch组件change回调事件是否正常

测试方式：
  * 默认switch组件为关闭状态，
  * 点击switch组件，判断回调事件中组件状态
  * 再次点击switch组件，判断回调事件中组件状态
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data : {
      width : '',
      height: '',
      resultTxt:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      click:function() {
        this.resultTxt = 'click response' 
      },
      change:function(e) {
        this.resultTxt = 'change response:' + e.value
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
  .input{
    width: 500px;
    height:100px;
    border-width: 1px;
  }
</style>