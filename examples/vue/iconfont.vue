<template>
  <scroller style='align-items:center'>
    <div style='margin-top:20px'>
      <text style="color:red;font-size:50px">only support font for ttf and woff</text>
    </div>
    <text class="title2" style='margin-top:50px;width:500px'>http ttf: &#xe606;&#xe605;&#xe604;&#xe603;&#xe602;&#xe601;&#xe600;</text>
    <text class="title3" style='margin-top:50px;width:500px'>http woff: &#xe606;&#xe605;&#xe604;&#xe603;&#xe602;&#xe601;&#xe600;</text>
    
    <div style='flex-direction:row;margin-top:50px'>
      <text style='font-family:iconfont4;font-size:50;color:green'>&#xe614;&#xe612;&#xe613;</text>
      <text style='font-family:iconfont4;font-size:50;'>&#xe614;&#xe612;&#xe613;&#xe61d;&#xe714;</text>
      <text style='font-family:iconfont4;font-size:60;color:blue'>&#xe711;</text>
      <text style='font-family:iconfont4;font-size:60;color:green'>&#xe71c;&#xe60b;</text>
    </div>
    <div style='flex-direction:row;margin-top:50px'>
      <text style='font-family:iconfont4;font-size:50;color:green'>&#xe71b;</text>
      <text style='font-family:iconfont4;font-size:50;'>&#xe713;</text>
      <text style='font-family:iconfont4;font-size:60;color:blue'>&#xe714;</text>
      <text style='font-family:iconfont4;font-size:60;color:green'>&#xe71f;&#xe602;&#xe715;&#xe60a;&#xe606;&#xe717;&#xe657;&#xe654;&#xe64f;&#xe637;</text>
    </div>
    <div>
      <text style='font-family:iconfont4;font-size:100; margin-top:50px'>&#xe631;&#xe630;&#xe627;&#xe63e;&#xe649;&#xe641;&#xe643;</text>
    </div>

    <div>
      <text style='font-family:iconfont4;font-size:100;color:green;margin-top:50px'>&#xe61e;&#xe600;&#xe62b;&#xe646;</text>
      <text style='font-family:iconfont4;font-size:100;margin-top:50px'> </text>
    </div>

    <div style='flex-direction:row'>
      <text style='font-family:iconfont4;font-size:70;width:750px'>&#xe748;&#xe747;&#xe746;&#xe746;&#xe745;&#xe744;&#xe742;&#xe740;&#xe73f;&#xe73a;&#xe731;&#xe6e0;&#xe72c;&#xe730;&#xe740;&#xe6e1;&#xe6d7;&#xe739;&#xe73d;&#xe735;&#xe72a;&#xe6e3;&#xe738;</text>
    </div>
    
  </scroller>
  
</template>
<style>
  .title1 {
    color: red;
    font-size: 36;
    font-family: iconfont1;
  }

  .title2 {
    color: gray;
    font-size: 36;
    font-family: iconfont2;
  }

  .title3 {
    color: pink;
    font-size: 36;
    font-family: iconfont3;
  }

  .title4 {
    color: red;
    font-size: 36;
    font-family: iconfont4;
  }
</style>

<script>
  module.exports = {
    created: function() {

      var domModule = weex.requireModule('dom');
      //目前支持ttf、woff文件，不支持svg、eot类型,moreItem at http://www.iconfont.cn/
      
      domModule.addRule('fontFace', {
        'fontFamily': "iconfont2",
        'src': "url('http://at.alicdn.com/t/font_1469606063_76593.ttf')"
      });
      domModule.addRule('fontFace', {
        'fontFamily': "iconfont3",
        'src': "url('http://at.alicdn.com/t/font_1469606522_9417143.woff')"
      });
      
        domModule.addRule('fontFace', {
        'fontFamily': "iconfont4",
        'src': "url('http://at.alicdn.com/t/font_zn5b3jswpofuhaor.ttf')"
      });


      // you can use the absolute path or the local:// scheme path
      //  domModule.addRule('fontFace', {
      //   'fontFamily': "iconfont4",
      //   'src': "url('file:///storage/emulated/0/Android/data/com.alibaba.weex/cache/http:__at.alicdn.com_t_font_1469606063_76593.ttf')"
      // });
      
    }
  }
</script>