<template>
	<list class="cList" @loadmore="onloadmore" @scroll="onscroll" 
		scrollable="true" loadmoreoffset=200>
		<refresh :display="refresh_display" @refresh="onrefresh" @pullingdown="onpullingdown" class="cRefresh">
			<text>↓ 上拉更新</text>
			<loading-indicator style="width:60;height:60"></loading-indicator>
		</refresh>
		<cell class="cCell" @appear="onappear" @disappear="ondisappear"
			v-for="(item,index) in items" :index="index" ref="cellItem">
			<text class="cText">{{item.name}}</text>
		</cell>
<!-- 		<loading :display="loading_display" @loading="onloading" @pullingup="onpullingup" class="cRefresh">
			<text>↑ 下拉加载更多</text>
			<loading-indicator style="width:60;height:60"></loading-indicator>
		</loading> -->
	</list>
</template>

<style type="text/css">
	.cList{
		height: 1280;
		overflow: hidden;
	}

	.cCell{
		height: 100;
		border-bottom-style: solid;
		border-bottom-width: 2;
		border-bottom-color: green;
	}

	.cRefresh{
		display: flex;
		flex-direction: column;
		justify-content: center;
		align-items: center;
		height: 200;
	}


</style>

<script type="text/javascript">

	var timeFunc = weex.requireModule("timer");
	module.exports = {

		data:function(){
			return {
				items:[],
				refresh_display:"hide",
				loading_display:"hide",

			}
		},

		created:function()
		{
			for(var i = 0; i < 30;i++)
			{
				var param = {name:"zhangwei" + i}
				this.items.push(param);
			}
		},

		methods:{
			onrefresh:function(e)
			{
				this.refresh_display = "show";
				var self = this;
				timeFunc.setTimeout(function(){
					self.refresh_display = "hide";
				},2000);
				console.log("======onrefresh")
			},

			onpullingdown:function(e)
			{
				console.log("onpullingdown--dy:" + e.dy + "," + e.pullingDistance + "," + e.viewHeight)
			},

			onloading:function(e)
			{
				this.loading_display = "show";
				var self = this;
				timeFunc.setTimeout(function(){
					self.loading_display = "hide";
				},2000);
				console.log("======onloading")
			},

			onpullingup:function(e)
			{
				console.log("onpullingup--dy:" + e.dy + "," + e.pullingDistance + "," + e.viewHeight)
			},

			onappear:function(e)
			{
				// console.log("onappear_" + e.target.attr.index)
			},

			ondisappear:function(e)
			{
				// console.log("ondisappear_"+ e.target.attr.index)
			},

			onloadmore:function()
			{
				console.log("onloadmore======")
				for(var i = 31; i < 60;i++)
				{
					var param = {name:"zhangwei" + i}
					this.items.push(param);
				}
			},

			onscroll:function()
			{
				console.log("11111")
			},

			dealKeyEvent:function(e)
			{
				var el = this.$refs.cellItem[10];
				weex.requireModule("dom").scrollToElement(el,{ offset:-300,animated:false });
			}


		}


	}
</script>