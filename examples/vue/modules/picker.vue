<template>
  <div class="conatiner">
    <div style="background-color: #286090">
      <text class="title" style="height: 80px ;padding: 20px;color: white">picker</text>
    </div>
	<div @click="normalpick" style="margin: 20px;padding:20px;background-color:#1ba1e2;color:#fff;"><text style="color:#fff">normalpick{{normalpickData}}</text></div>
	<div @click="pickDate" style="margin: 20px;padding:20px;background-color:#1ba1e2;color:#fff;"><text style="color:#fff">pickDate{{pickDateData}}</text></div>
	<div @click="pickTime" style="margin: 20px;padding:20px;background-color:#1ba1e2;color:#fff;"><text style="color:#fff">pickTime{{pickTimeData}}</text></div>
  </div>
</template>

<style>
  .container {
	flex: 1;
  }
</style>

<script>
  const picker = weex.requireModule('picker');
  module.exports = {
    data: {
      normalpickData:'',
      pickDateData:'',
         pickTimeData:''
    },
    methods: {
      normalpick: function() {
        var self = this;
    	picker.pick({
    	  height: '300px',
    	  items: [1, 2, 3, 4],
    	}, function(ret) {
    	  var result = ret.result;
    	  if (result == 'success') {
    		self.normalpickData = ret.data;
    	  }
    	})
      },
      pickDate: function(e) {
    	var self = this;
    	picker.pickDate({
    	  'value': '2016-12-20',
    	  'max': '2016-12-30',
    	  'min': '2014-12-30',
    	}, function(ret) {
    	  var result = ret.result;
    	  if (result == 'success') {
    		self.pickDateData = ret.data;
    	  }
    	});
      },
      pickTime:function(){
    	var self = this;
    	picker.pickTime({
    	}, function(ret) {
    	  var result = ret.result;
    	  if (result == 'success') {
    		self.pickTimeData = ret.data;
    	  }
    	});
      }
    }
  }
</script>