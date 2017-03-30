---
title: WebSocket
type: references
order: 3.11
version: 2.1
---

# WebSocket

## Summary

WebSockets is an advanced technology that makes it possible to open an interactive communication session between the user's H5/iOS/android and a server. With this API, you can send messages to a server and receive event-driven responses without having to poll the server for a reply

## **Notes:**
- iOS and h5 provide  webSocket default handle. if you use webSocket in android environment . you should provide custom adapter implementation,source:
  - [DefaultWebSocketAdapter.java](https://github.com/apache/incubator-weex/blob/dev/android/commons/src/main/java/com/alibaba/weex/commons/adapter/DefaultWebSocketAdapter.java);
  - [DefaultWebSocketAdapterFactory.java](https://github.com/apache/incubator-weex/blob/dev/android/commons/src/main/java/com/alibaba/weex/commons/adapter/DefaultWebSocketAdapterFactory.java);
  - refer:  [weex playground](https://github.com/apache/incubator-weex/tree/dev/android/playground)
- only support in .we file,not support in vue2.0

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
        <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">websocket</text>
      </div>
      <input
              type="text"
              placeholder="please input message to send"
              class="input"
              autofocus="false"
              value=""
              onchange="onchange"
              oninput="oninput"
              id = "input"
      />
      <div style="flex-direction: row; justify-content: center;">
        <text class="button" onclick="{{connect}}">connect</text>
        <text class="button" onclick="{{send}}">send</text>
        <text class="button" onclick="{{close}}">close</text>
      </div>
      <div style="background-color: #D3D3D3;margin-top: 20">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = send</text>
      </div>
      <text style="color: #000000;height: 80">{{sendinfo}}</text>
      <div style="background-color: #D3D3D3">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = onopen</text>
      </div>
      <text style="color: #000000;height: 80">{{onopeninfo}}</text>
      <div style="background-color: #D3D3D3">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = onmessage</text>
      </div>
      <text style="color: #000000;height: 100">{{onmessage}}</text>
      <div style="background-color: #D3D3D3">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = onclose</text>
      </div>
      <text style="color: #000000;height: 80">{{oncloseinfo}}</text>
      <div style="background-color: #D3D3D3">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = onerror</text>
      </div>
      <text style="color: #000000;height: 80">{{onerrorinfo}}</text>
      <div style="background-color: #D3D3D3">
        <text class="title" style="height: 80 ;padding: 20;color: #000000">method = close</text>
      </div>
      <text style="color: #000000;height: 80">{{closeinfo}}</text>
    </div>
    </div>
  </scroller>
</template>
<style>
  .input {
    font-size: 40;
    height: 80;
    width: 600;
  }
  .button {
    font-size: 36;
    width: 150;
    color: #41B883;
    text-align: center;
    padding-top: 10;
    padding-bottom: 10;
    border-width: 2;
    border-style: solid;
    margin-right: 20;
    border-color: rgb(162, 217, 192);
    background-color: rgba(162, 217, 192, 0.2);
  }
</style>
<script>
  var websocket = require('@weex-module/webSocket');
  module.exports = {
    data: {
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
      baseURL: '',
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
        var input = this.$el('input');
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
    }
  };
</script>
```

[Have a try](http://dotwe.org/weex/0256bbfc448f0fdbe8b10ba311c947b4)
