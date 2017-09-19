<template>
  <div>
    <panel title = 'input-event' :padding-body='0'>
      <div>
        <input test-id="input-obj" class="input mr-base" type="text" placeholder="onchange" @input="eventInput"></input>
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
      eventInput:function (e) {
        this.resultTxt='input:' + e.value
      },
      eventChange:function(e){
        this.resultTxt='change:'+ e.value 
      },  
      eventFocus:function (e) {
        this.resultTxt='focus:'+ e.value 
      }, 
      eventBlur:function (e) {
        this.resultTxt='blur:'+ e.value 
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