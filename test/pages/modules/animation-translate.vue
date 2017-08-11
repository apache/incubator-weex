<template>
  <div>
    <panel title = 'animation-translate' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="translate" @click.native="translate"></button>
        <button class='mr-base' type="info" size="middle" value="translateX" @click.native="translateX"></button>
        <button class='mr-base' type="info" size="middle" value="translateY" @click.native="translateY"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
        <text style="font-size:30px">top:{{top}}</text>
        <text style="font-size:30px">left:{{left}}</text>
      </panel>
    </panel>
    <div ref="block" class="block" :style="{'transform-origin':transformOrigin}">
        <text class="font" :value = 'testValue'></text>
    </div>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 

测试方式：
  * 
  * 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  const animation = weex.requireModule('animation');
  const dom = weex.requireModule('dom');

  module.exports = {
    data : {
      resultTxt:'',
      top:'',
      left:'',
      transformOrigin: 'center center',
      current_rotate: 0,
      current_scale: 1,
      current_color: '#FF0000',
      current_opacity: 1,
      current_translate: '',
      current_translateX:'',
      current_translateY:'',
      current_transform: '',
      isStop: true,
      testValue:"test"
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      anim: function (styles, timingFunction, duration, callback) {
        animation.transition(this.$refs.block, {
              styles: styles,
              timingFunction: timingFunction,
              duration: duration
          }, callback);
      },
      
      translate: function (e) {
        this.resultTxt=''
          this.current_translate = this.current_translate ? '' : 'translate(50%, 50%)';
          this.anim({
              transform: this.current_translate
          }, 'linear', 500, ()=>{
            this.resultTxt='translate结束'
            dom.getComponentRect(this.$refs.block, option => {
              console.log('getComponentRect:', option)
              this.top = option.size.top;
              this.left = option.size.left;
            })
          });
      },
      translateX: function (e) {
        this.resultTxt=''
          this.current_translateX = this.current_translateX ? '' : 'translateX(-400px)';
          this.anim({
              transform: this.current_translateX
          }, 'linear', 500, ()=>{
            this.resultTxt='translateX结束'
            dom.getComponentRect(this.$refs.block, option => {
              console.log('getComponentRect:', option)
              this.top = option.size.top;
              this.left = option.size.left;
            })
          });
      },
      translateY: function (e) {
        this.resultTxt=''
          this.current_translateY = this.current_translateY ? '' : 'translateY(-125px)';
          this.anim({
              transform: this.current_translateY
          }, 'linear', 500, ()=>{
            this.resultTxt='translateY结束'
            dom.getComponentRect(this.$refs.block, option => {
              console.log('getComponentRect:', option)
              this.top = option.size.top;
              this.left = option.size.left;
            })
          });
      },
    }
  }
</script>

<style scoped>
  .block {
        position: absolute;
        width: 300;
        height: 300;
        top: 350;
        left: 250;
        bottom:250;
        background-color: #FF0000;
    }
  .origin{background-color: #FFFFDF;width: 200;height: 50;}
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>