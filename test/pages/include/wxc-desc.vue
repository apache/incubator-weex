<template>
  <div class='absolute'>
    <div class='desc'>
      <button type="info" size="middle" value="测试描述" @click.native="show"></button>
    </div>
    <mask class="mask" v-if="shown"  @click="hide">
      <div class="content" @click="divclick">
        <text class='h3'>测试描述：</text>
        <text class='tips'>点击遮罩层可关闭!</text>
        <slot></slot>
      </div>
    </mask>
  </div>
</template>

<script>
  module.exports = {
    components: {
      button: require('./button.vue')
    },
    data : {
      shown:false
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
    left:20;
    top:20;
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

