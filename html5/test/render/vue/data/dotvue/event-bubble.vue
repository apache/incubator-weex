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
