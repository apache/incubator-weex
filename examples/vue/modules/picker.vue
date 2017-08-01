<template>
    <scroller>
        <div title="picker module" type="primary">
            <text style="margin-bottom: 20px;">pick value: {{value}}</text>
            <text type="primary" class="button" @click="pick" value="single pick" style="margin-bottom: 20px;"></text>
            <text type="primary" class="button" @click="pickDate" value="pickDate" style="margin-bottom: 20px;"></text>
            <text type="primary" class="button" @click="pickTime" value="pickTime"></text>
        </div>

        <div title="input component" type="primary">
            <text>onchange: {{txtChange}}</text>
            <input type="date" placeholder="select date" class="input" autofocus="false" value="" @change="onchange" max="2029-11-28" min="2015-11-28"/>
            <input type="time" placeholder="select time" class="input" autofocus="false" value="" @change="onchange"/>
        </div>
    </scroller>
</template>

<style scoped>
    .input {
        font-size: 60px;
        height: 80px;
        width: 400px;
    }
    .button {
        font-size: 36;
        width: 200;
        color: #41B883;
        text-align: center;
        padding-top: 10;
        padding-bottom: 10;
        border-width: 2;
        border-style: solid;
        margin-right: 20;
        border-color: rgb(162, 217, 192);
        background-color: rgba(162, 217, 192, 0.2);
    }
</style>

<script>
    module.exports = {

        data: function () {
            return {
                value: '',
                index: '',
                txtChange: ''
            };
        },
        methods: {
            pick: function () {
                var picker = require('@weex-module/picker');
                var items = new Array("Saab", "Volvo", "BMW");
                var self = this;
                picker.pick({
                    'items': items,
                    'index': self.index
                }, function (ret) {
                    var result = ret.result;
                    if (result == 'success') {
                        self.value = items[ret.data];
                        self.index = ret.data;
                    }
                });
            },
            pickDate: function () {
                var picker = require('@weex-module/picker');
                var self = this;
                picker.pickDate({
                    'value': '2016-11-28',
                    'max': '2029-11-28',
                    'min': '2015-11-28'
                }, function (ret) {
                    var result = ret.result;
                    if (result == 'success') {
                        self.value = ret.data;
                    }
                });
            },
            pickTime: function () {
                var picker = require('@weex-module/picker');
                var self = this;
                picker.pickTime({
                    'value': '19:24'
                }, function (ret) {
                    var result = ret.result;
                    if (result == 'success') {
                        self.value = ret.data;
                    }
                });
            },
            onchange: function (event) {
                this.txtChange = event.value;
                console.log('onchange', event.value);
            }
        }
    };
</script>
