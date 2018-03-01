<template>
  <div>
    <panel title = 'dom-update' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="push" @click.native="push"></button>
        <button class='mr-base' type="info" size="middle" value="concat" @click.native="concat"></button>
        <button class='mr-base' type="info" size="middle" value="unshift" @click.native="unshift"></button>
      </div>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="pop" @click.native="pop"></button>
        <button class='mr-base' type="info" size="middle" value="shift" @click.native="shift"></button>
      </div>
      <panel title='校验结果：'>
        <div v-for="txt in textList">
          <text class="font">{{txt}}</text>
        </div>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 

测试方式：
  * concat->push->unshift->shift->pop 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data :function(){
      return {
        textList:[],
        textList2:[]
      }
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      push:function() {
        this.textList.push('List-' + this.textList.length)
      },
      concat:function() {
        this.textList = this.textList.concat(this.textList2)
      },
      unshift:function() {
        this.textList.unshift('List-unshift-' + this.textList.length)
      },
      pop:function() {
        this.textList.pop()
      },
      shift:function() {
        this.textList.shift()
      }
    },
    created: function() {
      for(var i = 0; i<5; i++) {
        this.textList2.push('List2-' + this.textList2.length)
      }
    }
  }
</script>

<style scoped>
  .font{font-family: Times New Roman;font-size:20}
  .origin{background-color: #FFFFDF;width: 200;height: 50;}
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>