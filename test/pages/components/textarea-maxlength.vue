<template>
  <scroller class="wrapper">
    <text test-id="status" style="font-size:30">{{value}}</text>
    <textarea test-id="textarea" id="textarea" class="textarea" value="" autofocus="true" maxlength=4 @input="oninput" @change="onchange" ></textarea>
  </scroller>
</template>
<script>
  const modal = weex.requireModule('modal')
  export default {
    data () {
      return {
        value:'1'
      }
    },
    methods: {
      oninput (event) {
        this.value = event.value
        console.log('oninput:', event.value)
      },
      onchange (event) {
        console.log('onchange:', event.value)
      },
    }
  }
</script>
<style>
  .textarea {
    font-size: 50px;
    width: 650px;
    margin-top: 50px;
    margin-left: 50px;
    padding-top: 20px;
    padding-bottom: 20px;
    padding-left: 20px;
    padding-right: 20px;
    color: #666666;
    border-width: 2px;
    border-style: solid;
    border-color: #41B883;
    maxlength:10;
  }
</style>
