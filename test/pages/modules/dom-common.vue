<template>
  <div>
    <panel title = 'dom-common' :padding-body='0'>
      <div v-if="show">
        <text class="font">show text</text>
      </div>
      <div>
        <text class="font">{{jsonObj.firstName}}-{{jsonObj.lastName}}</text>
        <text class="font">{{runMethod()}}</text>
      </div>
      <div>
        <text :style='styleObj'>styleChange</text>
        <text :class="[activeClass, errorClass]">classChange</text>
      </div>
      <div v-for="txt in textList">
        <text class="font">{{txt}}</text>
      </div>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * dom相关操作

测试方式：
  * class暂不支持obj的写法，只支持数组写法
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  module.exports = {
    data :function(){
      return {
        textList:[],
        show:false,
        jsonObj:{
          firstName: 'Jack',
          lastName: 'Ma'
        },
        styleObj:{
          fontSize:30
        },
        activeClass:'font100', 
        errorClass:'colorRed'
      }
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    created: function() {
      this.promiseM().then(() =>{
        this.textList.push('promise support');
      });
      setTimeout(()=>{
        this.show = true;
        this.jsonObj.firstName = 'Zechang'
        this.jsonObj.lastName = 'Xie'
        this.styleObj.fontSize = 100;
        this.styleObj.color = 'red';
      },500);

    },
    methods: {
      promiseM: function() {
        return Promise.resolve('')
      },
      runMethod:function(){
        return 'runMethod';
      }
    }
  }
</script>

<style scoped>
  .font100{font-size: 100;}
  .font{font-family: Times New Roman;font-size:30}
  .origin{background-color: #FFFFDF;width: 200;height: 50;}
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>