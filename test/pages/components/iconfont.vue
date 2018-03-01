<template>
  <div>
    <panel title = 'iconfont' :padding-body='0'>
      <div style='flex-direction:row'> 
        <button class='mr-base' type="info" size="middle" value="change" @click.native="change"></button>
      </div>
      <panel title='校验结果：'>
        <text :style="{'font-size':'30px','font-family':fontF}">{{resultTxt}}</text>
      </panel>
    </panel>
    <wxc-desc>
      <text class='desc'>
测试点：
  * 验证iconfont是否正常展示

测试方式：
  * 截图比对
  * 点击按钮`change`，截图判断是否变化
      </text>
    </wxc-desc>
  </div>
</template>
<script>
	var he = require('he');
  module.exports = {
    data : {
    	fontF:'iconfont2',
      resultTxt:''
    },
    components: {
      "wxc-desc":require('../include/wxc-desc.vue'),
      panel: require('../include/panel.vue'),
      button: require('../include/button.vue'),
    },
    mounted: function () {
			var dom = weex.requireModule('dom');
			//目前支持ttf、woff文件，不支持svg、eot类型
			dom.addRule('fontFace', {
				'fontFamily': "iconfont2",
				'src': "url('http://at.alicdn.com/t/font_1469606063_76593.ttf')"
			});
			dom.addRule('fontFace', {
				'fontFamily': "iconfont3",
				'src': "url('http://at.alicdn.com/t/font_1469606522_9417143.woff')"
			});
			this.resultTxt=he.decode('http ttf: &#xe606;&#xe605;&#xe603;&#xe602;&#xe601;&#xe600;')
		},
    methods : {
      change:function() {
      	this.fontF='iconfont3';
        this.resultTxt = he.decode('http woff: &#xe606;&#xe605;&#xe604;&#xe603;&#xe602;&#xe601;&#xe600;') 
      },
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
  .input{
    width: 500px;
    height:100px;
    border-width: 1px;
  }
</style>