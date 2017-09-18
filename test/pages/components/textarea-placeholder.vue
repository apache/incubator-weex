<template>
  <div>
    <panel title = 'textarea-placeholder' :padding-body='0'>
      <div>
        <textarea test-id='textarea' type="text" :placeholder="placeholder" class="textarea" :value="value"></textarea>
      </div>
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
  module.exports = {
    data : {
      placeholder:'1111',
      value:'muti line'
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    created:function(){
      setTimeout(() => {
          this.placeholder = 'Hello,I am placeHolder';
        },1000);
    },
    methods : {
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
  .textarea {
    padding:10;
    font-size: 40;
    height: 280;
    border-width:1;
    border-style:solid;
    border-color: #dddddd;
    margin-top:30;
    margin-left:20;
    width: 600;
  }
</style>


