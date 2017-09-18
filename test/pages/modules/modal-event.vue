<template>
  <div>
    <panel title='modal-event'>
      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="toast" @click.native="toastClick"></button>
      </div>

      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="alert" @click.native="alertClick"></button>
      </div>

      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="confirm" @click.native="confirmClick"></button>
      </div>

      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="prompt" @click.native="promptClick"></button>
      </div>

      <panel title='校验结果：'>
        <text style="font-size:30px">{{toastResult}}</text>
        <text style="font-size:30px">{{alertResult}}</text>
        <text style="font-size:30px">{{confirmResult}}</text>
        <text style="font-size:30px">{{promptResult}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 测试modal的四个接口：toast、alert、confirm、prompt

测试方式：
  * 调用toast、alert、confirm、prompt，查看是否符合预期

      </text>
    </wxc-desc>
  </div>
</template>
<script>

  var modal = weex.requireModule("modal");
  module.exports = {
    data : {
      toastResult:'',
      alertResult:'',
      confirmResult:'',
      promptResult:'',
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      toastClick:function() {
        modal.toast({
          'message': 'message',
          'duration': 3,
        });
        this.toastResult = 'Toast success';
      },

      alertClick:function() {
        var params = {
          'message':'message',
          'okTitle':'OK',
        }
        modal.alert(params,()=>{
          this.alertResult = 'Alert success';
        });
      },

      confirmClick:function() {
        modal.confirm({
          'message': 'msg',
          'okTitle': 'ok',
          'cancelTitle': 'cancel',
        }, ()=>{
          this.confirmResult = "Confirm success";
        });
      },

      promptClick:function() {
        modal.prompt({
          'message': 'I am Prompt!',
          'okTitle': 'ok',
          'cancelTitle': 'cancel'
        }, ()=>{
          this.promptResult = "Prompt success";
        });
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
