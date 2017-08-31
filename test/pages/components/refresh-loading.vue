<template>
  <div>
    <panel title = 'refresh-loading' :padding-body='0'>
      <div style='flex-direction:row'>
        <button class='mr-base' type="info" size="middle" value="hideRefresh" @click.native="hideRefresh"></button>
        <button class='mr-base' type="info" size="middle" value="hideLoading" @click.native="hideLoading"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">refresh:{{refreshDisplay}}</text>
        <text style="font-size:30px">loading:{{loadingDisplay}}</text>
      </panel>
    </panel>
    <list>
      <refresh  class="refresh" :display="refreshDisplay" @refresh="onrefresh">
        <loading-indicator class="indicator"></loading-indicator>
        <text class="refresh-txt">↓ 下拉刷新</text>
      </refresh>

      <cell v-for="(k,v) in cells" style="height:100">
        <text class="txt">{{k}}-{{v}}</text>
      </cell>

      <loading class="refresh" :display="loadingDisplay" @loading="onloading">
        <loading-indicator class="indicator"></loading-indicator>
        <text class="refresh-txt">↑ 上拉加载</text>
      </loading>
    </list>

    <wxc-desc>
      <text class='desc'>
测试点：
  * 测试list的refresh（下拉刷新）和loading（上拉加载）

测试方式：
  * 下拉list，触发refresh事件
  * refresh事件触发后，将refresh组件的display属性设为hide
  * 将list上拉到底部
  * 继续上拉list，触发loadmore事件
  * loadmore事件触发后，将loading组件的display属性设为hide
  
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data : {
      width : '',
      height: '',
      refreshDisplay: 'hide',
      loadingDisplay: 'hide',
      cells: [0,1,2,3,4,5,6,7,8,9],
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      onrefresh: function(e) {
        // show refresh
        this.refreshDisplay = 'show';
      },
      hideRefresh: function(e) {
        // hide refresh
        if (this.refreshDisplay !== 'hide') {
          this.refreshDisplay = 'hide';
        }
      },

      onloading: function() {
        // show
        this.loadingDisplay = 'show';

      },
      hideLoading:function() {
        if (this.loadingDisplay !== 'hide') {
          this.loadingDisplay = 'hide';
        }
      },
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
  .refresh{
    flex-direction:row;
    justify-content:center;
    align-items: center;
    width:750;
    height: 100;
    border-width:3;
    background-color: #33b5e5;
  }
  .refresh-txt {
    font-size: 30;
    color: #FFFF00;
    text-align: center;
  }
  .indicator{
    height: 50;
    width: 50;
    color: #889967;
  }
  .txt {
    font-size:40;
    height:100;
    padding:30;
    border-width:1;
    border-style:dotted;
  }
</style>
