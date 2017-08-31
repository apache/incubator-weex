<template>
        <scroller style="background-color: beige; width: 750px; height: 900px" offset-accuracy="10" @scroll="onscroll">
            <text style="font-size: 40px;position:fixed;top:0;left:0;" test-id="status">{{status}}</text>
            <div v-for="row in rows" style="padding: 10px;">
                <text style="width: 750px; height: 150px; background-color: aqua">
                    {{row.id}}
                </text>
            </div>
        </scroller>
</template>

<script>
    export default {
        data: function(){
            return {
            rows: [],
            status: '-'
            }
        },
        created: function () {
                for (var i = 0; i < 20; i++) {
                    this.rows.push({"id": "Row " + i});
                }
            },
        methods: {
            onscroll: function (e) {
                this.status = e.contentOffset.y;
            }
        }
    }
</script>