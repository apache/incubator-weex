<template>
  <div class="event-bubble-outter" @click="outterClick">
    <div>
      <div class="event-bubble-inner" @click="innerClick"></div>
    </div>
  </div>
</template>

<script>
  module.exports = {
    methods: {
      outterClick (evt) {
        if (this.$parent.$parent.tracker.indexOf(' > in-bar-outter-div') === -1) {
          this.$parent.$parent.tracker += ' > in-bar-outter-div'
        }
      },
      innerClick (evt) {
        if (this.$parent.$parent.tracker.indexOf(' > in-bar-inner-div') === -1) {
          this.$parent.$parent.tracker += ' > in-bar-inner-div'
        }
      }
    }
  }
</script>
