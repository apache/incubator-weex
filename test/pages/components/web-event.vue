<template>
  <div>
    <panel title = 'web-event' :padding-body='0'>
      <div style='flex-direction:row'> 
        <web ref='web-ref' class="mr-base web" :src = "src"  @pagestart = "pagestartEvt" @pagefinish = "pagefinishEvt" @error = "errorEvt"></web> 
      </div>
      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="reload" @click.native="reload"></button>
        <button class='mr-base' type="info" size="middle" value="goBack" @click.native="goBack"></button>
        <button class='mr-base' type="info" size="middle" value="goForward" @click.native="goForward"></button>
      </div>
      <panel title='校验结果：'>
        <text test-id='txtStart' style="font-size:30px">txtStart:{{txtStart}}</text>
        <text test-id='txtFinish' style="font-size:30px">txtFinish:{{txtFinish}}</text>
        <text test-id='txtError' style="font-size:30px">txtError:{{txtError}}</text>
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

  var web = weex.requireModule("webview");

  module.exports = {
    data : {
      src:'http://www.error.com',
      txtStart:'',
      txtFinish:'',
      txtError:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      pagestartEvt:function(e){
        console.log("pagestart-----:"+JSON.stringify(e))
        this.txtStart=e.url
      },
      pagefinishEvt:function(e){
        console.log("pagefinish-----"+JSON.stringify(e))
        this.txtFinish=e.url
      },
      errorEvt:function(e){
        console.log("error-----"+JSON.stringify(e))
        this.txtError=e.url
      },
      goBack:function(e){
        this.txtFinish='';
        web.goBack(this.$refs['web-ref']);
      },
      reload:function(e){
        this.txtFinish='';
        web.reload(this.$refs['web-ref']);
      },
      goForward:function(e){
        this.txtFinish='';
        web.goForward(this.$refs['web-ref']);
      }
    },
    created:function(){
      setTimeout(()=>{
        this.src = 'https://h5.m.taobao.com'
      },1000);
      setTimeout(()=>{
        this.src = 'https://www.baidu.com'
      },3000);
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
    width: 600px;
    height:400px;
    border-width: 1px;
  }
</style>