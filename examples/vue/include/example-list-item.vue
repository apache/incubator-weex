<template>
  <wxc-list-item v-on:click="redirect">
    <text class="item-txt">{{ title }}</text>
  </wxc-list-item>
</template>

<style>
  .item-txt {
    font-size: 48px;
    color: #555;
  }
</style>

<script>
  var event = require('@weex-module/event')
  module.exports = {
    components: {
      'wxc-list-item': require('./wxc-list-item.vue')
    },
    props: {
      title: {
        default: '456'
      },
      url: {
        default: ''
      }
    },
    methods: {
      redirect: function() {
        event.openURL(this.url)
      }
    }
  }
</script>
