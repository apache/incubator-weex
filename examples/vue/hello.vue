<template>
  <div>
    <div ref="foo" :style="{height: '200px'}" class="ct"></div>
  </div>
</template>

<style scoped>
.ct {
  width: 200px;
  flex: 1;
  flex-direction: row;
  transform: translate3d(100px, 100px, 0);
  background-color: red;
}
</style>

<script>
  module.exports = {
    mounted () {
      console.log(this.$refs.foo.$el)
    }
  }
</script>
