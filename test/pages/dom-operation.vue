<template>
    <div>
        <div style="height:100;background-color:blue">
            <text v-if="display">item</text>
        </div>
        <div>
            <div v-for="item in repeat_items">
                <text>repeat</text>
            </div>
        </div>
        <text @click="onclick">display</text>
        <text @click="onclick2">display</text>
        <text @click="onclick3">{{text}}</text>
    </div>
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