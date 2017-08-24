<template>
  <div>
    <panel title = 'a-support-href2' :padding-body='0'>
      <div style='flex-direction:row'>
        <button class='mr-base' type="info" test-id='just_text' size="middle" value="jump_back" @click.native="jump_back"></button>
      </div>
    </panel>
  </div>
</template>
<script>
  var navigator = weex.requireModule('navigator')
  var modal = weex.requireModule('modal')
  module.exports = {
    data : {
      href:'a-support-href1.js?_wxpage=true'
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      jump_back : function(event) {
        navigator.pop({
          animated: "true"
        }, event => {
          modal.toast({
            message: 'callback: ' + event
          })
        })
      }
    }
  }
</script>

<style scoped>
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
  .button {
    width: 450px;
    padding-top: 20px;
    padding-bottom: 20px;
    border-width: 2px;
    border-style: solid;
    border-color: #DDDDDD;
    background-color: #F5F5F5
  }
  .text {
    font-size: 60px;
    color: #666666;
    text-align: center;
  }
</style>