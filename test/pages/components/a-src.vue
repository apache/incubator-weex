<template>
  <div>
    <panel title = 'a-src' :padding-body='0'>
      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="changeSrc" @click.native="changeSrc"></button>
      </div>
      <div>
        <a test-id='a-itself' class="button mr-base" :href="href">
          <text test-id='content-inside-a' class="text">Jump</text>
        </a>
        <a class="button mr-base" href="">
          <text class="text">JumpBlank</text>
        </a>

        <button class='mr-base' type="info" test-id='just_text' size="middle" value="jump_back" @click.native="jump_back"></button>
      </div>

      <panel title='校验结果：'>
        <text test-id='href' style="font-size:30px">{{href}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * src跳转
  * src更新

测试方式：
  * 点击A标签，查看跳转是否正确
  * 更新src后点击A标签，查看跳转是否正确
      </text>
    </wxc-desc>
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
      changeSrc : function(e) {
        this.href = 'a-support-href2.js?_wxpage=true'
      },
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