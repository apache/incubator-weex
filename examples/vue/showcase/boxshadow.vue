
<template>
    <div class="wrapper">
        <div style="width:400px; height:60px;background-color: #FFE4C4; box-shadow:20px  10px rgb(255, 69, 0);">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
        <div style="margin-top: 80px;width:400px; height:60px;background-color: #FFE4C4; box-shadow: 20px  10px 5px rgba(255, 69, 0, 0.8);">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
        <div style="margin-top: 80px;width:400px; height:60px;background-color: #FFE4C4; box-shadow:inset 20px  10px 5px rgba(255, 69, 0, 0.8);">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
        <div style="margin-top: 80px;width:400px; height:60px;background-color: #FFE4C4; box-shadow:inset 20px  10px 5px rgb(255, 69, 0);">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
        <div style="margin-top: 80px;width:400px; height:60px;background-color: #FFE4C4; box-shadow:20px  10px 5px black;">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
        <div style="margin-top: 80px;width:400px; height:60px;background-color: #FFE4C4; box-shadow:20px  10px 5px #008B00;">
            <text class="title" style="text-align: center">Hello {{target}}</text>
        </div>
    </div>
</template>

<style scoped>
    .wrapper {align-items: center; margin-top: 120px;}
    .title {font-size: 48px;}
</style>

<script>
    module.exports = {
        data: function () {
            return {
                logoUrl: 'https://alibaba.github.io/weex/img/weex_logo_blue@3x.png',
                target: 'World'
            };
        }
    };
</script>
