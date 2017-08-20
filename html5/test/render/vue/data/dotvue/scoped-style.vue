<template>
  <div>
    <div ref="foo" style="height:200px;" :style="{ backgroundColor: 'red' }" class="ct"></div>
  </div>
</template>

<style scoped>
.ct {
  display: flex;
  width: 200px; /* width: 100 * 2 - test trimComment */
  flex-direction: row; /* flex-direction: 2 * 100 - test trimComment */
  transform: translate3d(100px, 100px, 0);
  background-image: linear-gradient(to top, #f5fefd, #ffffff);
}
</style>

<script>
  module.exports = {
    mounted () {
      setTimeout(() => {
        this.done('test-style')
      }, 1000)
    }
  }
</script>
