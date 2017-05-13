<template>
    <div class="cChangeChannel" :style='{visibility : isHiden}'>

        <text class="cChannelNum">{{channelNum}}</text>

        <div class="cProgramArea">
            <text class="cChannelName">{{channelName}}</text>
            <text class="cPlayingProgram">{{playingProgram}}</text>
        </div>
    </div>
</template>

<style>
    .cChangeChannel{
        width: 320px;
        height: 72px;

        display: flex;
        flex-direction: row;
        justify-content: flex-start;

        background-color: red;
    }

    .cChannelNum{
        font-size: 40px;
        background-color: rebeccapurple;
        align-self:center;
        border-right-style: solid;
        border-right-color: white;
        border-right-width: 2px;
        padding-right: 20;
        text-align: center;
    }


    .cProgramArea{
        display: flex;
        flex-direction: column;
        justify-content: center;
        background-color: green;
    }

    .cChannelName{
        font-size: 28px;
        background-color: yellow;
        margin: 0;
        padding: 0;
    }

    .cPlayingProgram{
        font-size: 13px;
        background-color: yellow;
    }


</style>

<script type="text/javascript">

    module.exports =  {

        props:{
            channelNum:{default:"003"},
            channelName:{default:"黑龙江卫视高清"},
            playingProgram:{default:"正在播出 06:00-06:45 二十二炮手(12)"},
            isHiden:{default:"hidden"}
        },

        methods:{
            printfInfo:function(){
                console.log("printfInfo  living=====")
            }
        }
    }
</script>