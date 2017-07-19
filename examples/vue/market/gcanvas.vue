<template>
  <div ref="test">
    <gcanvas ref="canvas_holder" style="width:750;height:750;"></gcanvas>
  </div>
</template>
<script>
  //	var gcanvas = weex.requireModule('weex-gcanvas');

  var GCanvas=require('weex-gcanvas');
  var Image=require('weex-gcanvas/gcanvasimage');

  module.exports = {
    mounted: function () {
      var ref = this.$refs.canvas_holder;
      var gcanvas = GCanvas.start(ref)
      var ctx = gcanvas.getContext('2d');

      ctx.fillStyle = 'red';
      ctx.fillRect(0, 0, 100, 100);

      ctx.fillStyle = 'black';
      ctx.fillRect(100, 100, 100, 100);
      ctx.fillRect(25, 210, 700, 5);

      ctx.arc(450, 200, 100, 0, Math.PI * 2, true);
      ctx.fill();

      var img = new Image();
      img.onload = function(){
        ctx.drawImage(img, 100, 200, 210, 330);
        // ctx.drawImage(img, 0,0,105,165, 100, 200, 210, 330);
      };
      img.src = 'https://img.alicdn.com/tps/TB1TFNdKVXXXXbeaXXXXXXXXXXX-210-330.png';
    }
  };
</script>