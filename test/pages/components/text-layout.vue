<template>
  <scroller class="wrapper">
    <text test-id ="text-only">Hello World</text>
    <text test-id ="lines" style="lines:2;line-height:32px;font-size:36px;color:#0000FF">永和九年，岁在癸丑，暮春之初，会于会稽山阴之兰亭。\n\t群贤毕至，少长咸集。此地有崇山峻领，茂林脩竹；又有清流激湍，暎带左右。引以为流觞曲水，列坐其次。\n\tLet us not seek to satisfy our thirst for freedom. \n\tAgain and again, we must rise to the majestic heights of meeting physical force with soul force.</text>
    <text test-id ="font" style="lines:3;font-size:48px;color:#FF0000">其形也，翩若惊鸿，婉若游龙，荣曜秋菊，华茂春松。髣髴兮若轻云之蔽月，飘飖兮若流风之回雪。远而望之，皎若太阳升朝霞。迫而察之，灼若芙蕖出渌波。秾纤得衷，修短合度。肩若削成，腰如约素。延颈秀项，皓质呈露，芳泽无加，铅华弗御。云髻峨峨，修眉联娟，丹唇外朗，皓齿内鲜。明眸善睐，靥辅承权，瓌姿艳逸，仪静体闲。柔情绰态，媚于语言。奇服旷世，骨像应图。披罗衣之璀粲兮，珥瑶碧之华琚。戴金翠之首饰，缀明珠以耀躯。践远游之文履，曳雾绡之轻裾。微幽兰之芳蔼兮，步踟蹰于山隅。于是忽焉纵体，以遨以嬉。左倚采旄，右荫桂旗。攘皓腕于神浒兮，采湍濑之玄芝。</text>
    <text test-id ="fixed-size" style="width:300px; height:100px;color:#FF00FF">壬戌之秋，七月既望，苏子与客泛舟游于赤壁之下。清风徐来，水波不兴。举酒属客，诵明月之诗，歌窈窕之章。少焉，月出于东山之上，徘徊于斗牛之间。白露横江，水光接天。纵一苇之所如，凌万顷之茫然。浩浩乎如冯虚御风，而不知其所止；飘飘乎如遗世独立,羽化而登仙。</text>
    <div style="flex-direction:row;width:500px;height:300px;background-color:red">
      <text style="flex:1;line-height:36px;background-color:#AC34Ef" test-id="flexgrow-alignitems">Hello </text>
    </div>

    <div style="flex-direction:row;width:500px;height:300px;background-color:red;align-items:center">
      <text style="flex:1;line-height:40px;background-color:#12CDEE" test-id="flexgrow">World</text>
    </div>

    <div style="width:500px;height:300px;background-color:red">
      <text style="flex:1;line-height:36px;background-color:blue" test-id="flexgrow-alignitems-coloumn">Hello </text>
    </div>

    <div style="width:500px;height:300px;background-color:red;align-items:center">
      <text style="flex:1;line-height:40px;background-color:blue" test-id="flexgrow-column">World</text>
    </div>
  </scroller>
</template>

<style>
.wrapper{
    width: 750px;
    background-color: yellow;
}
</style>