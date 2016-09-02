<template>
  <example-list :items="items" dir="examples"></example-list>
</template>

<script>
  module.exports = {
    components: {
      'example-list': require('./include/example-list.vue')
    },
    data: {
      items: [
        // `name` key is the example filename without '.vue'
        {name: 'vue/syntax/hello-world', title: 'Hello World'},
        {name: 'vue/showcase/itemlist', title: 'List'},
        {name: 'vue/showcase/calculator', title: 'Calculator'}
      ]
    }
  }
</script>
