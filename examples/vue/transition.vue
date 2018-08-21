<template>
    <list class="list">
        <cell class="cell" v-for="(item, i) in panels" @click="change(i)">
            <div class="panel" :style="{height: item.height,width:item.width,backgroundColor:item.bgc,opacity:item.opacity,transform:item.transform}">
                 <text class="text">{{item.label}}</text> 
            </div>
            <div class="panel" :style="{height: item.height,width:item.width,backgroundColor:item.bgc,opacity:item.opacity,transform:item.transform}">
                 <text class="text">{{item.label}}</text> 
            </div>
        </cell>
    </list>
</template>
<script>
    export default {
        created () {
            this.datasource = ['width+height','backgroundColor','opactity','transform/translateX','transform/translateY','transform/scaleX','transform/scaleY','transform/rotate']
            this.panels = this.randomfn()
        },
        data () {
            return {
                panels: []
            }
        },
        methods: {
            change (i) {
                const item = this.panels[i]
                if (item) {

                    if(i==0)
                    {
                        item.height = item.height === 330 ? 200 : 330
                        item.width = item.width === 350 ? 730 : 350
                    }
                    if(i==1)
                    {
                        item.bgc = item.bgc ===  '#69BE96' ? 'red':'#69BE96'
                    }
                    if(i==2)
                    {
                        item.opacity = item.opacity === 1.0 ? 0.6 : 1.0
                    }
                    if(i==3)
                    {
                        item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                        item.transform = item.transform === 'translateX(200px)'?'translateX(0px)':'translateX(200px)'
                    }
                    if(i==4)
                    {
                        item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                        item.transform = item.transform === 'translateY(-150px)'?'translateY(0px)':'translateY(-150px)'
                    }
                    if(i==5)
                    {
                       item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                       item.transform = item.transform === 'scaleX(0.5)'?'scaleX(1)':'scaleX(0.5)'
                    }
                    if(i==6)
                    {
                       item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                       item.transform = item.transform === 'scaleY(0.5)'?'scaleY(1)':'scaleY(0.5)'
                    }
                    if(i==7)
                    {
                       item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                       item.transform = item.transform === 'rotate(40deg)'?'rotate(0deg)':'rotate(40deg)'
                    }
                    
                }
            },
            randomfn () {
                let ary = [];
                for(let i = 0; i<= 7; i++) {
                    ary.push({label: this.datasource[i] ,height: 200 , width:730, bgc:'#69BE96',opacity:1,right:20})
                }
                return ary;
            },
        }
    }
</script>

<style scoped>
    .panel {
        margin: 10px;
        top:10px;
        align-items: center;
        justify-content: center;
        border: solid;
        border-radius: 10px; 
          
        transition-property: right,left,top,bottom,width,height,backgroundColor,opacity,transform;
        transition-duration: 0.5s; 
        transition-delay: 0s;
        transition-timing-function: cubic-bezier(0.58, 0.1, 0.42, 1.0); 
    }
    .cell{
        background-color:white;
        flex-direction: row;
    }


    .text {
        font-size: 60px; 
        text-align: center;
        color: white;

    }
    .list{
        background-color:white;
    }
</style>
