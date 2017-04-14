<template>
  <div>
    <foo @appear="appear" @disappear="disappear"></foo>
  </div>
</template>

<script>
  module.exports = {
    components: {
      foo: require('./first-screen-appear-foo.vue')
    },
    methods: {
      appear: function (evt) {
        window._spy_first_screen_appear(evt)
      },
      disappear: function (evt) {
        window._spy_first_screen_appear(evt)
      }
    }
  }
</script>
