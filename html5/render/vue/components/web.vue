<template>
  <iframe :src="src" class="weex-web"></iframe>
</template>

<script>
  export default {
    props: {
      src: String
    }
  }
</script>
