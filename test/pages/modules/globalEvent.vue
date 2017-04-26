<template>
  <div style="flex-direction:row">
    <text test-id="content" class='btn' v-if="show">{{content}}</text>
    <text test-id="button" class="btn" @click="handleOnclick">{{btnTitle}}</text>
  </div>
</template>

<script>
  module.exports = {
    data() {
      return {
        content:"I received event",
        show:false,
        btnTitle:"click me to fire globalEvent"
      }
    },
    mounted(){
      var self = this;
      weex.requireModule("globalEvent").addEventListener("showText", function(e){
          console.log("Hello")
          self.show = true;
      });
    },
    methods:{
      handleOnclick:function(e) {
        var self = this;
        weex.requireModule("event").fireNativeGlobalEvent("showText",function(e){
            if(Boolean(e.ok)) {
              self.btnTitle = "posted"
            }
        })

      }
    }
  }
</script>

<style>
.btn {
  margin-top:100;
  margin-left:100;
  border-width:3;
  height:50;
  border-color:red;
  border-radius:10;
  background-color:rgba(23,145,245,1)
}
</style>