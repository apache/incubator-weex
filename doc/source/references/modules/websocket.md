---
title: WebSocket
type: references
order: 3.11
version: 2.1
---

# WebSocket
<span class="weex-version">v0.12+</span>

## Summary

WebSockets is an advanced technology that makes it possible to open an interactive communication session between the user's H5/iOS/android and a server. With this API, you can send messages to a server and receive event-driven responses without having to poll the server for a reply

## **Notes:**
- iOS and h5 provide  webSocket default handle. if you use webSocket in android environment . you should provide custom adapter implementation,source:
  - [DefaultWebSocketAdapter.java](https://github.com/apache/incubator-weex/blob/dev/android/commons/src/main/java/com/alibaba/weex/commons/adapter/DefaultWebSocketAdapter.java);
  - [DefaultWebSocketAdapterFactory.java](https://github.com/apache/incubator-weex/blob/dev/android/commons/src/main/java/com/alibaba/weex/commons/adapter/DefaultWebSocketAdapterFactory.java);
  - refer:  [weex playground](https://github.com/apache/incubator-weex/tree/dev/android/playground)

## API
### `WebSocket(url, protocol)`

create websocket

#### Arguments

- `url {string}`:The URL to which to connect;
- `protocol {string}`:the websocket protocol

### `send(data)`

Transmits data to the server over the WebSocket connection

#### Arguments

- `data {string}`:A text string to send to the server.

### `close(code,reason)`

Closes the WebSocket connection or connection attempt, if any. If the connection is already CLOSED, this method does nothing.

#### Arguments

- `code {number}`: the status code explaining why the connection is being closed.
- `reason {string}`:a string explaining why the connection is closing

### `onopen(options)`

An event listener to be called when the WebSocket connection's readyState changes to OPEN; this indicates that the connection is ready to send and receive data.

#### Arguments

- `options {object}`:an empty object

### `onmessage(options)`

An event listener to be called when a message is received from the server

#### Arguments

- `options {object}`:the server message options
  - `data {string}`: The listener received message

### `onclose(options)`

An event listener to be called when the WebSocket connection's readyState changes to CLOSED

#### Arguments

- `options {object}`:the CloseEvent is sent to clients using WebSockets when the connection is closed
  - `code {number}`: Returns an unsigned short containing the close code send by the server
  - `reason {string}`: Returns a string indicating the reason the server closed the connection
  - `wasClean {boolen}`: Returns a Boolean that Indicates whether or not the connection was cleanly closed.

### `onerror(options)`

An event listener to be called when an error occurs.

#### Arguments

- `options {object}`:the error event
  - `data {string}`: The listener received error data

### Example

```html
<template>
  <scroller>
    <div>
      <div style="background-color: #286090">
        <text class="title" style="height: 80px ;padding: 20px;color: white">websocket</text>
      </div>
      <input type="text" placeholder="please input message to send" class="input" autofocus="false" value="" @change="onchange" @input="oninput" ref="input"/>
      <div style="flex-direction: row; justify-content: center;">
        <text class="button" @click="connect">connect</text>
        <text class="button" @click="send">send</text>
        <text class="button" @click="close">close</text>
      </div>

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

      <div style="background-color: lightgray">
        <text class="title" style="height: 80px ;padding: 20px;color: black">method = close</text>
      </div>
      <text style="color: black;height: 80px">{{closeinfo}}</text>

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
    data () {
      return {
        connectinfo: '',
        sendinfo: '',
        onopeninfo: '',
        onmessage: '',
        oncloseinfo: '',
        onerrorinfo: '',
        closeinfo: '',
        txtInput:'',
        navBarHeight: 88,
        title: 'Navigator',
        dir: 'examples',
        baseURL: ''
      }
    },
    methods: {
      connect:function() {
        websocket.WebSocket('ws://echo.websocket.org','');
        var self = this;
        self.onopeninfo = 'connecting...'
        websocket.onopen = function(e)
        {
          self.onopeninfo = 'websocket open';
        }
        websocket.onmessage = function(e)
        {
          self.onmessage = e.data;
        }
        websocket.onerror = function(e)
        {
          self.onerrorinfo = e.data;
        }
        websocket.onclose = function(e)
        {
          self.onopeninfo = '';
          self.onerrorinfo = e.code;
        }
      },
      send:function(e) {
        var input = this.$refs.input;
        input.blur();
        websocket.send(this.txtInput);
        this.sendinfo = this.txtInput;
      },
      oninput: function(event) {
        this.txtInput = event.value;
      },
      close:function(e) {
        websocket.close();
      },
    },
  }
</script>
```

[Have a try](http://dotwe.org/vue/6d8bdfe66f24fda1a2dc6158b0182573)
