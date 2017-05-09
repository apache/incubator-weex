<template>
  <div class="ct">
    <bar @click="click"></bar>
    <text class="txt">{{tracker}}</text>
  </div>
</template>

<script>
  module.exports = {
    data: {
      tracker: ''
    },
    updated () {
      this.done('test-event-bubble', this.tracker)
    },
    methods: {
      click (e) {
        this.tracker += ' > component-bar'
      }
    },
    components: {
      bar: require('./event-bubble-bar.vue')
    }
  }
</script>

<style scoped>
.ct {
  height: 600px;
  background-color: #fff;
}
.txt {
  font-size: 32px;
  padding: 20px;
}
</style>
