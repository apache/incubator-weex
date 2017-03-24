<template>
    <div>
        <scroller style="background-color: beige; width: 750px; height: 900px" offset-accuracy="10" @scroll="onscroll">
            <div v-for="row in rows" style="padding: 10px;">
                <text style="width: 750px; height: 150px; background-color: aqua">
                    {{row.id}}
                </text>
            </div>
        </scroller>
        <text style="font-size: 40px;">{{status}}</text>
    </div>
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