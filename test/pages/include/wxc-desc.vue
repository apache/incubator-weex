<template>
  <div class='absolute'>
    <div class='desc'>
      <button type="info" size="middle" value="测试描述" @click.native="show"></button>
    </div>
    <div class="mask" :style='maskStyle' v-if="shown"  @click="hide">
      <div class="content" @click="divclick">
        <text class='h3'>测试描述：{{hello}}</text>
        <text class='tips'>点击遮罩层可关闭!</text>
        <slot></slot>
      </div>
    </div>
  </div>
</template>

<script>
  module.exports = {
    mounted:function(){
      var h = weex.config.env.deviceHeight;
      this.maskStyle.height = (h-100) +'px'
      this.maskStyle.top = (250 - h) +'px'
    },
    components: {
      button: require('./button.vue')
    },
    data: function () {
      return {
        shown:false,
        maskStyle:{
          left:'-430px',
          top:'-1100px',
          height:'0px'
        },
        hello:''
      }
    },
    methods : {
      show : function(e) {
        this.shown = true
        this.$forceUpdate()
      },
      hide: function(e) {
        this.shown = false
        this.$forceUpdate()
      },
    }
  }
</script>

<style scoped>
  .mask{
    background-color: #cccccc;
    position:absolute;
    left:-430;
    top:-1100;
    width:710;
  }
  .content{
    margin:20;
  }
  .h3 {font-size: 36px; font-weight: bold;
    margin-top: 20px; margin-bottom: 20px;
  }
  .desc{
    margin:20;
  }
  .tips{
    font-size: 13px;
    position: absolute;
    right:10px;
    top:10px;
  }
  .absolute{
    position:absolute;
    right:20;
    bottom:20;
  }
</style>

