<template>
  <div style="padding: 23px">
    <div class="segment">
      <text class="segment-title">OnScroll & OnChange Event</text>
      <div class="debug-panel" style="margin: 10px">
        <text test-id='s1_p'>{{progress}}</text>
        <text test-id='s1_f'>{{flag}}</text>
        <text test-id='s1_d'>{{direction}}</text>
        <text test-id='s1_i'>{{index}}</text>
      </div>
      <div style="height: 60px; flex-direction: row;">
        <div test-id='s1_page0' style="flex: 1;background-color: darkcyan;justify-content: center;align-items: center"
             @click="goto(0)">
          <text class="page-title">Page 1</text>
        </div>
        <div test-id='s1_page1' style="flex: 1;background-color: aquamarine;justify-content: center;align-items: center"
             @click="goto(1)">
          <text class="page-title">Page 2</text>
        </div>
        <div test-id='s1_page2' style="flex: 1;background-color: darkcyan;justify-content: center;align-items: center"
             @click="goto(2)">
          <text class="page-title">Page 3</text>
        </div>
      </div>
      <div style="height: 10px;background-color: skyblue">
        <div :style="{ width: '233px', height: '10px', marginLeft: progress, backgroundColor: 'black' }"></div>
      </div>
      <slider test-id='slider_1' ref="slider_1" class="slider" interval="4500" @change="onchange" append="tree"
              :index="index"
              @scroll="onscroll" offset-x-accuracy="0.01">
        <div class="frame" v-for="img in imageList">
          <image class="image" resize="cover" :src="img.src"></image>
          <text class="title">{{img.title}}</text>
        </div>
        <indicator style="height: 20px"></indicator>
      </slider>
    </div>
    <div class="segment" style="margin-top: 20px">
      <text class="segment-title">Infinite Scroll = true</text>
      <div class="debug-panel">
        <text test-id='s2_switch' :style="{ height: '48px', padding: '10px', backgroundColor: bg1, fontWeight: 'bold' }"
              @click="toggle(1, true)">{{status1}}
        </text>
        <div style="flex-direction: row">
          <text style="padding: 10px;">switch count:</text>
          <text test-id='s2_count' style="font-size: 40px;margin-right: 10px;color: green">{{count1}}</text>
        </div>
      </div>
      <slider test-id='slider_2' class="slider" :auto-play="autoPlay1" interval="500" @change="update(1)">
        <div class="frame" v-for="img in imageList2">
          <image class="image" resize="cover" :src="img.src"></image>
          <text class="title">{{img.title}}</text>
        </div>
      </slider>
    </div>

    <div class="segment" style="margin-top: 20px">
      <text class="segment-title">Infinite Scroll = false</text>
      <div class="debug-panel">
        <text test-id='s3_switch' :style="{ height: '48px', padding: '10px', backgroundColor: bg2, fontWeight: 'bold' }"
              @click="toggle(2, true)">{{status2}}
        </text>
        <div style="flex-direction: row">
          <text style="padding: 10px;">switch count:</text>
          <text test-id='s3_count' style="font-size: 40px;margin-right: 10px;color: green">{{count2}}</text>
        </div>
      </div>
      <slider test-id='slider_3' class="slider" infinite="false" :auto-play="autoPlay2" interval="500"
              @change="update(2)">
        <div class="frame" v-for="img in imageList2">
          <image class="image" resize="cover" :src="img.src"></image>
          <text class="title">{{img.title}}</text>
        </div>
        <indicator style="height: 20px"></indicator>
      </slider>
    </div>
  </div>
</template>
<style scoped>
  .page-title {
    color: black;
    font-size: 30px;
    font-weight: bold;
  }

  .segment-title {
    font-size: 30px;
    background-color: lightgray;
    padding: 10px;
  }

  .segment {
    border-color: black;
    border-width: 2px;
  }

  .image {
    width: 700px;
    height: 200px;
  }

  .slider {
    width: 700px;
    height: 200px;
  }

  .title {
    position: absolute;
    top: 20px;
    left: 20px;
    padding-left: 20px;
    width: 200px;
    color: #FFFFFF;
    font-size: 36px;
    line-height: 60px;
    background-color: rgba(0, 0, 0, 0.3);
  }

  .frame {
    width: 700px;
    height: 200px;
  }

  .debug-panel {
    flex-direction: row;
    justify-content: space-between;
  }
</style>
<script>
  module.exports = {
    data: {
      imageList: [{
        title: 'Page 1',
        src: 'https://gd2.alicdn.com/bao/uploaded/i2/T14H1LFwBcXXXXXXXX_!!0-item_pic.jpg'
      }, {
        title: 'Page 2',
        src: 'https://gd1.alicdn.com/bao/uploaded/i1/TB1PXJCJFXXXXciXFXXXXXXXXXX_!!0-item_pic.jpg'
      }, {
        title: 'Page 3',
        src: 'https://gd3.alicdn.com/bao/uploaded/i3/TB1x6hYLXXXXXazXVXXXXXXXXXX_!!0-item_pic.jpg'
      }],
      imageList2: [{
        title: 'Page 1',
        src: 'https://gd2.alicdn.com/bao/uploaded/i2/T14H1LFwBcXXXXXXXX_!!0-item_pic.jpg'
      }, {
        title: 'Page 2',
        src: 'https://gd1.alicdn.com/bao/uploaded/i1/TB1PXJCJFXXXXciXFXXXXXXXXXX_!!0-item_pic.jpg'
      }],
      index: 0,
      progress: 0,
      ratio: 0,
      direction: 'unknown',
      count: 0,
      switchCount: 0,
      flag: false,
      autoPlay1: false,
      autoPlay2: false,
      bg1: 'greenyellow',
      bg2: 'greenyellow'
      ,
      status1: 'PLAY'
      ,
      status2: 'PLAY'
      ,
      count1: 0
      ,
      count2: 0
    },
    methods: {
      goto(i) {
        this.index = i;
        this.progress = i * 233;
      },
      onscroll(e) {
        var ratio = parseFloat(e.offsetXRatio);
        if (Math.abs(ratio) > 0.05) {
          if (ratio < 0) {
            this.direction = 'left';
          } else if (ratio > 0) {
            this.direction = 'right';
          }
        }
        this.progress = 233 * this.index + 233 * -ratio;
        this.count++;
        this.flag = this.count <= 100;
      },
      onchange(e) {
        this.goto(parseInt(e.index));
        this.switchCount++;
        this.count = 0;
      },
      update(index) {
        if (index == 1) {
          this.count1++;
        } else if (index == 2) {
          this.count2++;
        }
      },
      toggle(index, repeat) {
        var self = this;
        if (index == 1) {
          if (this.autoPlay1) {
            this.status1 = 'PLAY';
            this.bg1 = 'greenyellow';
          } else {
            this.status1 = 'STOP';
            this.bg1 = 'red';
          }
          this.autoPlay1 = !this.autoPlay1;
        } else if (index == 2) {
          if (this.autoPlay2) {
            this.status2 = 'PLAY';
            this.bg2 = 'greenyellow';
          } else {
            this.status2 = 'STOP';
            this.bg2 = 'red';
          }
          this.autoPlay2 = !this.autoPlay2;
        }

        if (repeat) {
          setTimeout(function () {
            self.toggle(index, false);
          }, 5000);
        }
      },
    },
  };
</script>