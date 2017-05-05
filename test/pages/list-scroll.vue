<template>
        <list style="background-color: beige; width: 750px; height: 900px" offset-accuracy="10" @scroll="onscroll">
            <header><text test-id="status" ref="status" style="font-size: 40px;">{{status}}</text></header>
            <cell v-for="(index, row) in rows" :key="index" style="padding: 10px;">
                <text style="width: 750px; height: 150px; background-color: aqua">
                    {{row.id}}
                </text>
            </cell>
        </list>
        
</template>

<script>
    module.exports = {
        data: function(){
            return {
                rows: [],
                status: '-'
            }
        },
        created: function () {
                for (var i = 0; i < 20; i++) {
                    this.rows.push({"id": "Cell " + i});
                }
            },
        methods: {
            
            onscroll: function (e) {
                this.status = e.contentOffset.y;
            }
        }
    }
</script>