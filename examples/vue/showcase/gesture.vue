<template>
  <div>
    <div class="ct"
      @longpress="handlePress"
      @swipe="handleSwipe"
      @panstart="handlePanStart"
      @panmove="handlePanMove"
      @panend="handlePanEnd"
      :style="{ top: top, left: left }">
        <text>{{text}}</text>
    </div>
  </div>
</template>

<style scoped>
.ct {
  position: absolute;
  width: 750px;
  height: 1000px;
  background-color: #f7f7f7;
}
</style>

<script>
  module.exports = {
    data: function () {
      return {
        text: 'no press.',
        top: '0px',
        left: '0px',
        flag: false,
      }
    },
    methods: {
      handlePress: function(e) {
        this.text = '->longpress(' + e.touch.pageX + ',' + e.touch.pageY + ')id:'+ e.touch.identifier
      },
      handleSwipe: function(e) {
        this.text += '->swipe(' + e.touch.pageX + ',' + e.touch.pageY + ')id:'+ e.touch.identifier
      },
      handlePanStart: function(e) {
        this.flag = false
        this.text = '->panstart(' + e.touch.pageX + ',' + e.touch.pageY + ')id:'+ e.touch.identifier
      },
      handlePanMove: function(e) {
        if (this.flag) {
          return
        }
        this.flag = true
        this.text += '->panmove(' + e.touch.pageX + ',' + e.touch.pageY + ')id:'+ e.touch.identifier
      },
      handlePanEnd: function(e) {
        this.text += '->panend(' + e.touch.pageX + ',' + e.touch.pageY + ')id:'+ e.touch.identifier
      }
    }
  }
</script>