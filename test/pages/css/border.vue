<template>
  <scroller>
    <div class="container">
        <text class="radius box solid"></text>
        <text class="topleft box solid"></text>
        <text class="topright box solid"></text>
        <text class="bottomleft box solid"></text>
        <text class="bottomright box solid"></text>
        <text>solid</text>
    </div>
    <div class="container">
        <text class="radius box dotted"></text>
        <text class="topleft box dotted"></text>
        <text class="topright box dotted"></text>
        <text class="bottomleft box dotted"></text>
        <text class="bottomright box dotted"></text>
        <text class="dotted dottedLarge"></text>
        <text>dotted</text>
    </div>
    <div class="container">
        <text class="radius box dashed"></text>
        <text class="topleft box dashed"></text>
        <text class="topright box dashed"></text>
        <text class="bottomleft box dashed"></text>
        <text class="bottomright box dashed"></text>
        <text class="dashed dashedLarge"></text>
        <text>dashed</text>
    </div>
    <div class="container">
        <text class="constrained1"></text>
        <text class="constrained2"></text>
        <text class="constrained1 dashed"></text>
        <text class="constrained2 dashed" ></text>
        <text class="constrained1 dotted" ></text>
        <text class="constrained2 dotted" ></text>
        <text>constrained</text>
    </div>
    <div class="container">
        <text class="radius differentWidth1"></text>
        <text class="radius differentWidth2"></text>
        <text class="radius differentWidth3"></text>
        <text class="radius differentWidth4" ></text>
        <text class="radius differentWidth5" ></text>
        <text>different width</text>
    </div>
     <div class="container">
        <text class="borderClipping1"></text>
        <text class="borderClipping2"></text>
        <text class="borderClipping3"></text>
        <text>border clipping</text>
    </div>
    <div class="container">
        <text class="boxShadow1"></text>
        <text class="boxShadow2"></text>
        <text class="boxShadow3"></text>
        <text test-id="test-text">box shadow</text>
    </div>
    <div class="container">
        <text class="transparent1"></text>
        <text class="transparent2"></text>
        <text class="transparent3"></text>
        <text class="transparent4"></text>
        <text>transparent</text>
    </div>
    <div class="container">
        <image class="box topleft" :src="img" @load="onload"></image>
        <image class="box topright" :src="img" @load="onload"></image>
        <image class="box bottomleft" :src="img" @load="onload"></image>
        <image class="box bottomright" :src="img" @load="onload"></image>
        <image class="box" :src="img" @load="onload"></image>
        <image class="circle" :src="img" @load="onload"></image>
        <text>image</text>
    </div>
  </scroller>
</template>

<style>
.container {
  flex-direction: row;
}
.box {
  width: 100px;
  height: 100px;
  margin: 2px;
  background-color: #ccc;
  border-width: 6px;
  border-color: #f00;
}
/* the different border-radii styles defined in CSS3 */
.radius {
  border-radius: 20px;
}
.topleft {
  border-top-left-radius: 20px; 
}
.topright {
  border-top-right-radius: 20px; 
}
.bottomright {
  border-bottom-right-radius: 20px; 
}
.bottomleft {
  border-bottom-left-radius: 20px; 
}
.dottedLarge {
  width: 100px;
  height: 100px;
  margin: 2px;
  background-color: #ccc;
  border-width: 21px;
  border-radius: 40px;
  border-left-color: rgba(0,0,255,1);
  border-bottom-color: rgba(0,128,0,0.5);
  border-right-color: rgba(0,128,0,0.5);
  border-top-color: #f00;
}
.dashedLarge {
  width: 100px;
  height: 100px;
  margin: 2px;
  background-color: #ccc;
  border-top-width: 19px;
  border-bottom-width: 19px;
  border-left-width: 38px;
  border-right-width: 38px;
  border-top-left-radius: 50px;
  border-top-right-radius: 50px;
  border-bottom-left-radius: 25px;
  border-bottom-right-radius: 25px;
  border-left-color: rgba(0,0,255,1);
  border-right-color: #f00;
  border-top-color: #f00;
  border-bottom-color: #f00;
}

/* the different border styles defined in CSS3 */
.dotted { 
  border-style: dotted;
}
.dashed {
  border-style: dashed;
}
.solid {
  border-style: solid;
}

.constrained1 {
  width: 100px;
  height: 40px;
  border-width: 10px ;
  border-color: #f00;
  border-top-left-radius: 15px;
  border-top-right-radius: 40px;
  border-bottom-left-radius: 40px;
  border-bottom-right-radius: 15px;
}
    
.constrained2 {
  width: 40px;
  height: 100px;
  margin-left: 10px;
  border-width: 10px ;
  border-color: #f00;
  border-top-left-radius: 15px;
  border-top-right-radius: 40px;
  border-bottom-left-radius: 40px;
  border-bottom-right-radius: 15px;
}

.differentWidth1
{
  width: 100px;
  height: 100px;
  margin: 2px;
  border-color: #f00;
  background-color: #ccc;
  border-top-width: 10px;
  border-right-width: 15px;
  border-bottom-width: 20px;
  border-left-width: 30px;
}
.differentWidth2
{
  width: 100px;
  height: 100px;
  margin: 2px;
  border-color: #f00;
  background-color: #ccc;
  border-top-width: 10px;
  border-right-width: 25px;
  border-bottom-width: 40px;
  border-left-width: 25px;
}
.differentWidth3
{
  width: 100px;
  height: 100px;
  margin: 2px;
  border-color: #f00;
  background-color: #ccc;
  border-top-width: 10px;
  border-right-width: 25px;
  border-bottom-width: 10px;
  border-left-width: 25px;
}
.differentWidth4
{
  width: 100px;
  height: 100px;
  margin: 2px;
  border-color: #f00;
  background-color: #ccc;
  border-top-width: 16px;
  border-right-width: 4px;
  border-bottom-width: 110px;
  border-left-width: 32px;
}
.differentWidth5
{
  width: 100px;
  height: 100px;
  margin: 2px;
  border-color: #f00;
  background-color: #ccc;
  border-right-width: 50px;
  border-top-width: 6px;
  border-left-width: 6px;
  border-bottom-width: 6px;
}

.borderClipping1 {
  width: 100px;
  height: 100px;
  margin: 2px;
  background-color: #ccc;
  border-radius: 30px;
  border-top-color: red;
  border-top-width: 10px;
  border-left-color: blue;
  border-left-width: 10px;
  border-right-color: green;
  border-right-width: 30px;
  border-bottom-color: yellow;
  border-bottom-width: 30px;
}
.borderClipping2 {
  width: 100px;
  height: 100px;
  margin: 2px;
  background-color: #ccc;
  border-top-width: 20px;
  border-bottom-width: 20px;
  border-left-width: 40px;
  border-right-width: 40px;
  border-top-left-radius: 50px;
  border-top-right-radius: 50px;
  border-bottom-left-radius: 25px;
  border-bottom-right-radius: 25px;
  border-left-color: rgba(0,0,255,1);
  border-bottom-color: rgba(0,128,0,0.5);
  border-right-color: rgba(0,128,0,0.5);
  border-top-color: #f00;
}   
.borderClipping3 {
  background-color: #ccc;
  width: 100px;
  height: 100px;
  margin: 2px;
  border-width:10px;
  border-radius: 50px;
  border-top-right-radius: 0px;
  border-bottom-left-radius: 0px;
  border-left-color: rgba(0,0,255,1);
  background-color: green;
  border-top-width: 5px;
  border-bottom-width: 5px;
  border-left-width: 18px;
  border-right-width: 0px;
  border-top-color: #f00;
  border-bottom-color: #f00;
}

.boxShadow1 {
  width: 100px;
  height: 45px;
  background-color: #000000;
  border-top-left-radius: 30px;
  border-top-right-radius: 30px;
  box-shadow: 0px 0px 1px 1px #000000;
  margin-top: 20px;
}
.boxShadow2 {
  width: 100px;
  height: 45px;
  background-color: #000000;
  box-shadow: 30px 15px 0px -10px lime;
  border-bottom-right-radius: 50px;
  margin:30px;
}
.boxShadow3 {
  width: 100px;
  height: 45px;
  background-color: #000000;
  box-shadow: 0px 0px 1px 1px rgba(0,0,0,0.06);
  border-bottom-right-radius: 50px;
  margin:30px;
}

.circle {
  width: 100px;
  height: 100px;
  border-width: 33px;
  border-color: rgba(255, 0, 0, 0.5);
  border-style: solid;
  border-radius:50px;
}
.transparent1 {
  width: 100px;
  height: 100px;
  border-width: 33px;
  border-top-color: rgba(255, 0, 0, 0.5);
  border-left-color: rgba(255, 0, 0, 0.5);
  border-bottom-color: rgba(255, 0, 0, 0.5);
  border-style: solid;
  border-radius:50px;
  border-right-color: transparent;
}
.transparent2 {
  width: 100px;
  height: 100px;
  border-width: 33px;
  border-top-color: rgba(255, 0, 0, 0.5);
  border-left-color: rgba(255, 0, 0, 0.5);
  border-bottom-color: rgba(255, 0, 0, 0.5);
  border-style: solid;
  border-radius:50px;
  border-right-color: transparent; 
}
.transparent3 {
  width: 100px;
  height: 100px;
  border-top-width: 33px;
  border-left-width: 33px;
  border-right-width: 33px;
  border-top-color: rgba(255, 0, 0, 0.5);
  border-left-color: rgba(255, 0, 0, 0.5);
  border-style: solid;
  border-radius:50px;
  border-right-color: transparent; 
  border-right-width: 0;
  border-bottom-color: transparent;
  border-bottom-width: 0;
}
.transparent4 {
  width: 100px;
  height: 100px;
  border-top-width: 33px;
  border-left-width: 33px;
  border-bottom-width: 33px;
  border-top-color: rgba(255, 0, 0, 0.5);
  border-left-color: rgba(255, 0, 0, 0.5);
  border-style: solid;
  border-radius:50px;
  border-right-color: transparent; 
  border-right-width: 0;
  border-bottom-color: transparent;
}
</style>

<script>
  module.exports = {
    data : function(){
      return {
        img: 'http://gw.alicdn.com/tfs/TB1bg8SQFXXXXcHXXXXXXXXXXXX-85-170.png'
      }
    },
    methods : {
      onload: function(){
        console.log("image onload")
      }
    }
  }
</script>