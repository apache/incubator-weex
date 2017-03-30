<template>
    <div>
        <text>hello world.</text>
        <input ref="input" value="input" @blur="onblur"></input>
        <text style="color:red">{{status}}</text>
        <text @click="onclick">button</text>
        <text @click="toggleblur">blur input</text>
    </div>
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
