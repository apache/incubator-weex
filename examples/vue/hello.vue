<template>
    <div class="root">
        <div class="anim" ref="anim">
            <image class="award-box" ref="awardBox" src="https://img.alicdn.com/tps/TB1qsoYPpXXXXXcaXXXXXXXXXXX-718-777.png"></image>
        </div>
    </div>
</template>
<style scoped>
    .root {
        /*overflow: visible;*/
        position: absolute;
        left: 0px;
        right: 0px;
        top: 0px;
        bottom: 0px;
        background-color: rgba(0, 0, 0, 0.8);
        flex-direction: column;
        justify-content: center;
        align-items: center;
    }

    .anim {
        /*overflow: visible;*/
        position: absolute;
        left: 0px;
        right: 0px;
        top: 0px;
        bottom: 0px;
        flex-direction: row;
        justify-content: center;
        align-items: center;
    }

    .award-box {
        position: absolute;
        top: 300px;
        left: 125px;
        width: 498px;
        height: 540px;
    }
</style>
<script>
    const animation = weex.requireModule('animation');
    module.exports = {
        mounted: function () {
            var testEl = this.$refs.awardBox;
            animation.transition(testEl, {
                styles: {
                    transform: 'translate(250px, 100px)',
                    transformOrigin: 'center center'
                },
                duration: 800, //ms
                timingFunction: 'ease',
                delay: 500 //ms
            }, null);
        }
    }
</script>