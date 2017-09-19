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
                    item.height = item.height === 200 ? 400 : 200
                    item.width = item.width === 330 ? 660 : 330
                    item.bgc = item.bgc ===  '#69BE96' ? '#72B8DF':'#69BE96'
                    item.opacity = item.opacity === 1.0 ? 0.6 : 1.0
                    item.transform = item.transform === 'translateX(20px)'?'translateX(40px)':'translateX(20px)'
                }
            },
            randomfn () {
                let ary = [];
                for(let i = 1; i<= 10; i++) {
                    ary.push({label: i ,height: 200 , width:730, bgc:'#69BE96',opacity:1})
                }
                return ary;
            }
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
          
        transition-property: width,height,backgroundColor,opacity,transform;  
        transition-duration: 0.5s; 
        transition-delay: 0s;
        transition-timing-function: cubic-bezier(0.25, 0.1, 0.25, 1.0); 
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
        flex-direction: row;
    }
</style>
