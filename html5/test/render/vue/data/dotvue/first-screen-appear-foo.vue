<template>
  <div v-if="show" style="width:200px;height:200px;background-color:red;"></div>
</template>

<script>
module.exports = {
  data () {
    return {
      show: false
    }
  },
  mounted () {
    setTimeout(() => {
      this.show = true
      setTimeout(() => {
        this.show = false
        setTimeout(() => {
          this.show = true
          setTimeout(() => {
            this.done('test-first-screen-appear')
          }, 25)
        }, 300)
      }, 300)
    }, 300)
  }
}
</script>
