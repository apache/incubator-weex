<template>
  <div>
    <text>{{x}}</text>
  </div>
</template>

<script>
  module.exports = {
    data: {
      x: 'xxx'
    },
    created: function () {
      console.log('CREATE', this.x)
      this.x = 'yyy'
      console.log('CREATED', this.x)
      setTimeout(function () {
        console.log('TIMEOUT START', this.x)
        console.log(JSON.stringify(this.$document.documentElement, null, 2))
        this.x = 'zzz'
        console.log('TIMEOUT END', this.x)
        console.log(JSON.stringify(this.$document.documentElement, null, 2))
      }.bind(this), 1000)
    }
  }
</script>
