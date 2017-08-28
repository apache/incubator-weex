<template>
    <scroller>
            <text v-if="display" test-id="maynotexist">item</text>
            <div v-for="item in repeat_items">
                <text :test-id="'item'+item">repeat item:{{item}}</text>
            </div>
        <text test-id="btn1" @click="onclick">display</text>
        <text test-id="status" @click="onclick2">display</text>
        <text test-id="status2" @click="onclick3">{{text}}</text>
    </scroller>
</template>
<script>
    module.exports = {
        data:function(){
            return {
            display:false,
            repeat_items:[1,2,3,4,5],
            text:"display"
            }
        },
        methods:{
          onclick:function(){
            this.display=true;
            this.repeat_items.push(6);
          },
          onclick2:function(){
            this.display = false;
            this.repeat_items.pop();
            this.text = "finished"
          }
        }
    }
</script>