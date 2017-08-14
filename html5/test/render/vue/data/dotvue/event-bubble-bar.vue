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
        this.$parent.$parent.tracker += ' > in-bar-outter-div'
      },
      innerClick (evt) {
        this.$parent.$parent.tracker += ' > in-bar-inner-div'
      }
    }
  }
</script>
