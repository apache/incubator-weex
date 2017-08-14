<template>
  <div>
    <scroller ref="container" class="container" @scroll="onscroll">
      <div test-id='div_1' style="position:fixed; top: 0; left: 100px;"><text test-id='text_1'>{{vp}}|{{p1}}|{{p2}}</text></div>
      <div v-for="item in items" style="background-color:yellow;height:300"><text >row</text></div>
      <div ref="panel" class="fixed-panel"></div>
      <div ref="panel2" class="fixed-panel2"></div>
    </scroller>
  </div>
</template>
<style>
  .container {
    width: 750;
  }
  .fixed-panel {
    position: fixed;
    bottom: 0;
    left: 0;
    height: 200;
    background-color: #ff0000;
    width: 200;
  }
  .fixed-panel2 {
    position: fixed;
    right: 0;
    top: 0;
    height: 200;
    background-color: #ff0000;
    width: 200;
  }
</style>
<script>
    const dom = weex.requireModule('dom')
    module.exports = {
        data:function(){
            return {
            items:[1,1,1,1,1,1,1,1],
            height:100,
            vp:-1,
            p1:-1,
            p2:-1
            }
        },
        mounted:function(){
            var self = this;
            dom.getComponentRect(this.$refs.container,function(data){
                self.vp = data.size.height;
            })
        },
        methods:{
            onscroll:function(e){
                var self = this;
                dom.getComponentRect(this.$refs.container,function(data){
                    self.vp = Math.round(data.size.height);
                });
                dom.getComponentRect(this.$refs.panel,function(data){
                    self.p1 = Math.round(data.size.top+data.size.height);
                });
                dom.getComponentRect(this.$refs.panel2,function(data){
                    self.p2 = data.size.top;
                });
                //should vp != 0 && p1 == vp && p2 ==0
            }
        }
    }
</script>