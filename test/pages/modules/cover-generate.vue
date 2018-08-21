<template>
  <div>
    <panel title = 'cover-generate' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="generate" @click.native="generate"></button>
      </div>
      <panel title='校验结果：'>
        <text style="font-size:30px">{{resultTxt}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 

测试方式：
  * 
  * 
      </text>
    </wxc-desc>
  </div>
</template>
<script>
  var ext = weex.requireModule("ext");
  module.exports = {
    data : {
      resultTxt:'',
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    methods : {
      generate:function() {
        ext.generateCover((ret)=>{
        	this.resultTxt = 'coverage success'
        });
      }
    }
  }
</script>

<style scoped>
  .origin{background-color: #FFFFDF;width: 200;height: 50;}
  .mr-base{
    margin: 10px;
  }
  .desc{
    color:#aa0000;
    font-size: 30px;
  }
</style>