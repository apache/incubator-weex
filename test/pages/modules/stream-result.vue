<template>
  <div>
    <panel title = 'stream-result' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="streamGet" @click.native="streamGet"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
        <text style="font-size:30px">{{detail}}</text>
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
  var stream = weex.requireModule("stream");
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
      streamGet:function() {
        this.streamFetch("GET",GET_URL);
      },
      streamFetch:function(stype,surl){
        var me = this;
        if(this.sendtype=="jsonp"){
          surl=this.JSONP_URL;
        }
        stream.fetch({
          method: stype,
          url: surl,
          type:this.sendtype,
          timeout : 10000
        }, function(ret) {
          nativeLog(JSON.stringify(ret)+ typeof(ret.data));
          if(stype=="HEAD"){
              console.log('get:'+ret);
              me.resultTxt = 'response:success'
              me.detail = ret.statusText;
          }else{
            if(!ret.ok){
              me.resultTxt = 'response:failed';
              me.detail = (typeof(ret.data)!='undefined'?JSON.stringify(ret.data):"");
            }else{
              me.resultTxt = 'response:success';
              me.detail = "data type:"+ typeof(ret.data) + ";  " +JSON.stringify(ret.data);
            }
          }
        },function(response){
          console.log('get in progress:'+JSON.stringify(response));
          me.resultTxt = JSON.stringify(response);
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