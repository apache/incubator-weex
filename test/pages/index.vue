<template>
    <scroller>
        <text test-id="title">hello world.</text>
        <input ref="input" test-id="input" value="input" @blur="onblur"></input>
        <text style="color:red" test-id="status">{{status}}</text>
        <text @click="onclick" test-id="button">button</text>
        <text @click="toggleblur" test-id="button2">blur input</text>
    </scroller>
</template>
<script>
    export default {
        data:function(){
            return {
                status:"___"
            }
        },
        methods:{
            onclick:function(e){
                this.status = "btn click."
            },
            toggleblur:function(e){
                this.$refs.input.blur();
                // this.$el("input").blur();
            },
            onblur:function(e){
                this.status = "input blur."
            }
        }
    }
</script>
