<template>
  <div>
    <div>
      <text style="font-size: 40px">oninput: {{txtInput}}</text>
      <text style="font-size: 40px">onchange: {{txtChange}}</text>
      <text style="font-size: 40px">onreturntype: {{txtReturnType}}</text>
      <text style="font-size: 40px">selection: {{txtSelection}}</text>

    </div>
    <scroller>
      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = text</text>
        </div>
        <input type="text" placeholder="Input Text" class="input" :autofocus=true value="" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = password</text>
        </div>
        <input type="password" placeholder="Input Password" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = url</text>
        </div>
        <input type="url" placeholder="Input URL" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = email</text>
        </div>
        <input type="email" placeholder="Input Email" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = tel</text>
        </div>
        <input type="tel" placeholder="Input Tel" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = time</text>
        </div>
        <input type="time" placeholder="Input Time" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = number</text>
        </div>
        <input type="number" placeholder="Input number" class="input" @change="onchange" @input="oninput"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input type = date</text>
        </div>
        <input type="date" placeholder="Input Date" class="input" @change="onchange" @input="oninput" max="2017-12-12" min="2015-01-01"/>
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = default</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="default" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = go</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="go" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = next</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="next" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = search</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="search" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = send</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="send" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>

      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input return-key-type = done</text>
        </div>
        <input type="text" placeholder="please input" return-key-type="done" class="input" @change="onchange" @return = "onreturn" @input="oninput" />
      </div>


      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">function focus() & blur()</text>
        </div>
        <div style="flex-direction: row;margin-bottom: 16px;justify-content: space-between">
          <text class="button" value="Focus" type="primary" @click="focus"></text>
          <text class="button" value="Blur" type="primary" @click="blur"></text>
        </div>

        <input type="text" placeholder="Input1" class="input" value="" ref="input1"/>
      </div>


      <div>
        <div style="background-color: #286090">
          <text class="title" style="height: 80 ;padding: 20;color: #FFFFFF">input selection</text>
        </div>
        <div style="flex-direction: row;margin-bottom: 16px;justify-content: space-between">
          <text class="button" value="setRange" type="primary" @click="setRange"></text>
          <text class="button" value="getSelectionRange" type="primary" @click="getSelectionRange"></text>
        </div>
        <input type="text"  ref="inputselection" placeholder="please input" value="123456789"  class="input" @change="onchange" @return = "onreturn" @input="oninput"/>
      </div>



    </scroller>
  </div>
</template>

<style scoped>
  .input {
    font-size: 60px;
    height: 80px;
    width: 750px;
  }
  .button {
    font-size: 36;
    width: 200;
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
  module.exports = {
    data: function () {
      return {
        txtInput: '',
        txtChange: '',
        txtReturnType: '',
        txtSelection:'',
        autofocus: false
      };
    },
    methods: {
      ready: function () {
        var self = this;
        setTimeout(function () {
          self.autofocus = true;
        }, 1000);
      },
      onchange: function (event) {
        this.txtChange = event.value;
        console.log('onchange', event.value);
      },
      onreturn: function (event) {
        this.txtReturnType = event.returnKeyType;
        console.log('onreturn', event.type);
      },
      oninput: function (event) {
        this.txtInput = event.value;
        console.log('oninput', event.value);
      },
      focus: function () {
        this.$refs['input1'].focus();
      },
      blur: function () {
        this.$refs['input1'].blur();
      },
      setRange: function() {
        console.log(this.$refs["inputselection"]);
        this.$refs["inputselection"].setSelectionRange(2, 6);
      },
      getSelectionRange: function() {
        console.log(this.$refs["inputselection"]);
        var self = this;
        this.$refs["inputselection"].getSelectionRange(function(e) {
          self.txtSelection = e.selectionStart +'-' + e.selectionEnd;
        });
      }
    }
  };
</script>
