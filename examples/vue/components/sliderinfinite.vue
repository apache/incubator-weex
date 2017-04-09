<template>
  <div>
    <slider class="slider" interval="3000" auto-play="true">
      <div class="frame" v-for="img in imageList">
        <image class="image" resize="cover" :src="img.src"></image>
      </div>
    </slider>
  </div>
</template>

<style scoped>
  .image {
    width: 700px;
    height: 300px;
  }
  .slider {
    margin-top: 25px;
    margin-left: 25px;
    width: 700px;
    height: 300px;
    border-width: 2px;
    border-style: solid;
    border-color: #41B883;
  }
  .frame {
    width: 700px;
    height: 300px;
    position: relative;
  }
</style>

<script>
  export default {
    data () {
      return {
        imageList: [
          { src: 'https://gw.alicdn.com/tfs/TB1SA2wQXXXXXXDapXXXXXXXXXX-1400-600.png'},
          { src: 'https://gw.alicdn.com/tfs/TB1KS_OQXXXXXcSXVXXXXXXXXXX-1400-600.png'},
          { src: 'https://gw.alicdn.com/tfs/TB1Ez7XQXXXXXbNXXXXXXXXXXXX-1400-600.png'},
          { src: 'https://gw.alicdn.com/tfs/TB1XLn6QXXXXXb2XpXXXXXXXXXX-1400-600.png'}
        ]

      }
    }
  }
</script>