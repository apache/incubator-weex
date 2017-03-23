<template>
  <div class="wrapper">
    <text style="font-size:30">test textarea maxlenght</text>
    <textarea id="textarea" class="textarea" value="" maxlength=4 @input="oninput" @change="onchange" ></textarea>
  </div>
</template>
<script>
  const modal = weex.requireModule('modal')
  export default {
    methods: {
      oninput (event) {
        console.log('oninput:', event.value)
        modal.toast({
          message: `oninput: ${event.value}`,
          duration: 0.8
        })
      },
      onchange (event) {
        console.log('onchange:', event.value)
        modal.toast({
          message: `onchange: ${event.value}`,
          duration: 0.8
        })
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