<template>
  <scroller>
    <div>
      <div style="background-color: #286090">
        <text class="title" style="height: 80px ;padding: 20px;color: white">websocket</text>
      </div>
      <input type="text" placeholder="please input message to send" class="input" autofocus="false" value="" @input="oninput" ref="input" />
      <div style="flex-direction: row; justify-content: center;">
        <text class="button" @click="connect">connect</text>
        <text class="button" @click="send">send</text>
        <text class="button" @click="sendArrayBuffer">sendArrayBuffer</text>
        <text class="button" @click="close">close</text>
      </div>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = close</text>
      </div>
      <text style="color: black;height: 80px">{{closeinfo}}</text>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = send</text>
      </div>
      <text style="color: black;height: 80px">{{sendinfo}}</text>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = onopen</text>
      </div>
      <text style="color: black;height: 80px">{{onopeninfo}}</text>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = onmessage</text>
      </div>
      <text style="color: black;height: 400px">{{onmessage}}</text>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = onclose</text>
      </div>
      <text style="color: black;height: 80px">{{oncloseinfo}}</text>
      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = onerror</text>
      </div>
      <text style="color: black;height: 80px">{{onerrorinfo}}</text>
    </div>
  </scroller>
</template>

<style scoped>
  .input {
    font-size: 40px;
    height: 80px;
    width: 600px;
  }
  
  .button {
    font-size: 36px;
    width: 150px;
    color: #41B883;
    text-align: center;
    padding-top: 25px;
    padding-bottom: 25px;
    border-width: 2px;
    border-style: solid;
    margin-right: 20px;
    border-color: rgb(162, 217, 192);
    background-color: rgba(162, 217, 192, 0.2);
  }
</style>

<script>
  var websocket = weex.requireModule('webSocket')
  export default {
    data() {
      return {
        connectinfo: '',
        sendinfo: '',
        onopeninfo: '',
        onmessage: '',
        oncloseinfo: '',
        onerrorinfo: '',
        closeinfo: '',
        txtInput: '',
        navBarHeight: 88,
        title: 'Navigator',
        dir: 'examples',
        baseURL: ''
      }
    },
    methods: {

      connect: function() {
        websocket.WebSocket('ws://echo.websocket.org', '');
        var self = this;
        self.onopeninfo = 'connecting...'
        websocket.onopen = function(e) {
          self.onopeninfo = 'websocket open';
        }
        websocket.onmessage = function(e) {
          console.log(typeof(e.data));
          if(typeof(e.data) === 'string'){
            self.onmessage = e.data;
          }else
          {
            var str = 'receive array buffer show with string:' +  String.fromCharCode.apply(null, new Float32Array(e.data));
            self.onmessage = str;
          }
        }
        websocket.onerror = function(e) {
          self.onerrorinfo = e.data;
        }
        websocket.onclose = function(e) {
          self.onopeninfo = '';
          self.oncloseinfo = e.code;
        }
      },
      send: function(e) {
        var input = this.$refs.input;
        input.blur();
        websocket.send(this.txtInput);
        this.sendinfo = this.txtInput;
      },
      sendArrayBuffer: function(e) {
        var input = this.$refs.input;
        input.blur();
        var buffer = new ArrayBuffer(16)
        var view = new Float32Array(buffer)
        view.set([4,89,36.9,0.765])
        console.log(buffer);
        var str = 'send array buffer show with string:' +  String.fromCharCode.apply(null, new Float32Array(buffer));
        this.sendinfo = str;
        websocket.send(buffer);
      },
      oninput: function(event) {
        this.txtInput = event.value;
      },
      close: function(e) {
        this.closeinfo = 'close connect';
        websocket.close();
      },
    },
  }
</script>