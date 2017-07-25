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
        console.log(evt,'outter-div')
        this.$parent.$parent.tracker += ' > in-bar-outter-div'
      },
      innerClick (evt) {
        console.log(evt,'inner-div')
        this.$parent.$parent.tracker += ' > in-bar-inner-div'
      }
    }
  }
</script>
