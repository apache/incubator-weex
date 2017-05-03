<template>
  <div>
    <text style="font-size:100px;" test-id='just_text' @click="jump_back">You just clicked A tag.</text>
  </div>
</template>

<script>
  var navigator = weex.requireModule('navigator')
  export default {
    methods: {
      jump_back(event) {
        navigator.pop({
          animated: "true"
        }, event => {
          modal.toast({
            message: 'callback: ' + event
          })
        })
      }
    }
  };
</script>