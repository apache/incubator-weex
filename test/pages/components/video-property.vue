<template>
  <div>
    <panel title = 'video-property' :padding-body='0'>
      <div>
        <video class="video" :play-status="dataSource.playStatus"  :auto-play="dataSource.autoPlay" :src = "dataSource.src" @start = "start" @pause = "pause" @finish = "finish" @fail = "fail"></video>
      </div>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="setSrc" @click.native="setSrc"></button>
        <button class='mr-base' type="info" size="middle" value="setPlay" @click.native="setPlay"></button>
        <button class='mr-base' type="info" size="middle" value="setPause" @click.native="setPause"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * video属性更新

测试方式：
  * 
  * 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data: function(){
      return{
        dataSource: {
            autoPlay: false,
            playStatus: 'pause',
            src: 'http://test.mp4'
        },
        resultTxt:''
      }
    },
    methods: {
        start:function(e){
            console.log('video-start');
            this.resultTxt = "video-start";
        },
        pause:function(){
            console.log('video-pause');
            this.resultTxt = "video-pause";
        },
        finish:function() {
            console.log('video-finish');
            this.resultTxt = "video-finish";
        },
        fail:function() {
            console.log('video-fail');
            this.resultTxt = "video-fail";
        },
        setSrc: function () {
            console.log('setSrc');
            this.dataSource.src="http://g.tbcdn.cn/ali-wireless-h5/res/0.0.6/toy.mp4";
        },
        setPlay:function(){
            console.log('setPlay');
            this.dataSource.playStatus='play';
        },
        setPause:function(){
            console.log('setPause');
            this.dataSource.playStatus='pause';
        },
        setStop:function(){
            console.log('setStop');
            this.dataSource.playStatus='stop';
        },
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    }
  }
</script>

<style scoped>
  .video {
    width: 710; 
    height: 300;
    border-style: solid;
    border-width:1;
    border-color:#ff0000;
  }
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>
