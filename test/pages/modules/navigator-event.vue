<template>
  <navpage
    dataRole="none"
    :height="navBarHeight"
    :title="title"
    backgroundColor="#ff5898"
    titleColor="white"
    leftItemTitle="Left"
    leftItemColor="white"
    rightItemTitle="Right"
    @naviBarLeftItemClick="naviBarLeftItemClick"
    @naviBarRightItemClick="naviBarRightItemClick">
    <panel title="navigator-event">
      <div style='flex-direction:row;margin:10px;'>
        <button type="primary" v-if='!isPop' size="small" value="push" @click.native="push"></button>
        <button type="primary" v-if='isPop' size="small" value="pop" @click.native="pop"></button>
        <button type="primary" v-if='isPop' size="small" value="open" @click.native="open"></button>
        <button type="primary" v-if='isPop' size="small" value="close" @click.native="close"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{textResult}}</text>
      </panel>
    </panel>
    
    <wxc-desc>
      <text class='desc'>
  测试点：
  * 
  * 
  * 

  测试方式：
  * 
  * 
      </text>
    </wxc-desc>
  </navpage>
</template>

<script>
  var navigator = weex.requireModule('navigator')
  module.exports = {
    data: function () {
      return {
        navBarHeight: 88,
        title: 'Navigator',
        dir: 'examples',
        baseURL: '',
        subPath: weex.config.env.platform === 'Web' ? 'vue-web/' : '',
        textResult:'',
        isPop: false
      }
    },
    components: {
      panel: require('../include/panel.vue'),
      navpage: require('../include/navpage.vue'),
      button: require('../include/button.vue'),
      "wxc-desc":require('../include/wxc-desc.vue'),
    },
    created: function() {
      this.isPop = weex.config.bundleUrl.includes('type=pop')
      this.textResult = weex.config.bundleUrl;
    },
    methods: {
      naviBarLeftItemClick: function (e) {
        this.textResult='naviBarLeftItemClick';
      },
      naviBarRightItemClick: function (e) {
        this.textResult='naviBarRightItemClick';
      },
      push: function () {
        var params = {
          'url':  weex.config.bundleUrl + '?type=pop',
          'animated' : 'true',
        }
        navigator.push(params, function () {});
      },
      pop: function () {
        var params = {
          'url':  weex.config.bundleUrl.replace('?type=pop',''),
          'animated' : 'true',
        }
        navigator.pop(params, function () {});
      },
      open: function () {
        var params = {
          'url':  weex.config.bundleUrl.replace('?type=pop',''),
          'animated' : 'true',
        }
        navigator.open(params, function () {},function () {});
      },
      close: function () {
        var params = {
          'animated' : 'true',
        }
        navigator.close(params, function () {},function () {});
      },

    }
  }
</script>
