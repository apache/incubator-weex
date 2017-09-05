<template>
  <scroller>
    <panel title = 'stream-result' :padding-body='0'>
      <div style='flex-direction:row'>
        <button test-id='streamGet' class='mr-base' type="info" size="middle" value="streamGet" @click.native="streamGet"></button>
        <text style="font-size:30px">{{complete}}</text>
      </div>
      <panel title='校验结果：GET'>
        <text style="font-size:30px">{{getResult}}</text>
      </panel>

      <panel title='校验结果：GET_JSONP'>
        <text style="font-size:30px">{{getJSONPResult}}</text>
      </panel>

      <panel title="校验结果：POST">
        <text style="font-size:30px">{{postResult}}</text>
      </panel>

      <panel title="校验结果：PUT">
        <text style="font-size:30px">{{putResult}}</text>
      </panel>

      <panel title="校验结果：DELETE">
        <text style="font-size:30px">{{deleteResult}}</text>
      </panel>

      <panel title="校验结果：HEAD">
        <text style="font-size:30px">{{headResult}}</text>
      </panel>

      <panel title="校验结果：PATCH">
        <text style="font-size:30px">{{patchResult}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * stream模块的网络请求功能是否正常（GET、POST、PUT、DELETE、HEAD、PATCH）

测试方式：
  * 截图比对网络请求返回的结果

      </text>
    </wxc-desc>
  </scroller>
</template>
<script>
  var GET_URL = 'http://httpbin.org/get';
  var GET_URL_JSONP = 'http://jsfiddle.net/echo/jsonp/?callback=anything&result=content_in_response';
  var POST_URL = 'http://httpbin.org/post';
  var PUT_URL = 'http://httpbin.org/put';
  var DELETE_URL = 'http://httpbin.org/delete';
  var HEAD_URL = 'http://httpbin.org/status/418';
  var PATCH_URL = 'http://httpbin.org/patch';
  var stream = weex.requireModule("stream");

  module.exports = {
    data : {
      sendtype:'json',
      getResult:'',
      getJSONPResult:'',
      postResult:'',
      putResult:'',
      deleteResult:'',
      headResult:'',
      patchResult:'',
      complete:'',
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      streamGet:function() {
        var me = this;

        stream.fetch({
          method: "GET",
          url: GET_URL,
          type: this.sendtype,
          timeout : 10000
        }, function(ret) {
          if("GET"=="HEAD"){
              me.getResult = ret.resultTxt_GET;
          }else{
            if(!ret.ok){
              me.getResult = "request failed";
            }else{
              me.getResult = "data type:"+ typeof(ret.data) + ";  " +JSON.stringify(ret.data);
            }
          }
        },function(response){
          me.getResult = JSON.stringify(response);
        });


        stream.fetch({
          method: 'GET',
          url: GET_URL_JSONP,
          type:'jsonp'
        }, function(ret) {
          if(!ret.ok){
            me.getJSONPResult = "request failed";
          }else{
            me.getJSONPResult = JSON.stringify(ret.data);
          }
        },function(response){
          me.getJSONPResult = "bytes received:"+response.length;
        });

        stream.fetch({
          method: 'POST',
          url: POST_URL,
          type:'json',
          body:JSON.stringify({username:'weex'})//or you can just use JSON Object {username:'weex'}
        }, function(ret) {
          if(!ret.ok){
            me.postResult = "request failed";
          }else{
            me.postResult = JSON.stringify(ret.data);
          }
        },function(response){
          me.postResult = "bytes received:"+response.length;
        });


        stream.fetch({
          method: 'PUT',
          url: PUT_URL,
          type:'json'
        }, function(ret) {
          if(!ret.ok){
            me.putResult = "request failed";
          }else{
            me.putResult = JSON.stringify(ret.data);
          }
        },function(response){
          me.putResult = "bytes received:"+response.length;
        });

        stream.fetch({
          method: 'DELETE',
          url: DELETE_URL,
          type:'json'
        }, function(ret) {

          if(!ret.ok){
            me.deleteResult = "request failed";
          }else{
            me.deleteResult = JSON.stringify(ret.data);
          }
        },function(response){
          me.deleteResult = "bytes received:"+response.length;
        });

        stream.fetch({
          method: 'HEAD',
          url: HEAD_URL,
          type:'json'
        }, function(ret) {
          if(ret.statusText !== 'I\'m a teapot'){
            me.headResult = "request failed";
          }else{
            me.headResult = ret.statusText;
          }
        },function(response){
          me.headResult = "bytes received:"+response.length;
        });

        stream.fetch({
          method: 'PATCH',
          url: PATCH_URL,
          type:'json'
        }, function(ret) {
          if(!ret.ok){
            me.patchResult = "request failed";
            me.complete = "Completed";
          }else{
            me.patchResult = JSON.stringify(ret.data);
            me.complete = "Completed";
          }
        },function(response){
          me.patchResult = "bytes received:"+response.length;
          me.complete = "Completed";
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
