<template>
    <div>
        <text>{{d1}}</text>
        <slider auto-play="true" @change="change" style="width: 750px;height: 400px;background-color: blue" interval="500" infinite="false">
            <div style="flex: 1;background-color: beige;align-items: center;" v-for="item in items">
                <text style="font-size: 60px;">
                    {{item}}
                </text>
            </div>
            <indicator style="height: 20px"></indicator>
        </slider>
        
        <text style="margin-top:60">{{d2}}</text>
        <slider auto-play="true" @change="change2" style="width: 750px;height: 400px;background-color: blue;" interval="500">
            <div style="flex: 1;background-color: beige;align-items: center;" v-for="item in items">
                <text style="font-size: 60px;">
                    {{item}}
                </text>
            </div>
            <indicator style="height: 20px"></indicator>
        </slider>
    </div>
</template>

<script>
    var itemCount = 2;
    module.exports = {
        data: function(){
                return {
                    d1:0,
                    d2:0,
                    items: [],
                }
        },
        created: function () {
                for (var index = 0; index < itemCount ; index++) {
                    this.items[index] = 'Page: ' + index;
                }
        },
        methods: {
            change: function(e){
              this.d1 = parseInt(this.d1) + 1;
            },
            change2: function(e) {
              this.d2 = parseInt(this.d2) + 1;
            }
        }
    }
</script>