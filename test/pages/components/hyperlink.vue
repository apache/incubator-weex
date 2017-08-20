<template>
  <div class="wrapper">
    <a test-id='a-itself' class="button" href="hyperlink_target.js?_wxpage=true">
      <text test-id='content-inside-a' class="text">Jump</text>
    </a>
  </div>
</template>
<style>
  .wrapper {
    flex-direction: column;
    justify-content: center;
  }

  .button {
    width: 450px;
    margin-top: 30px;
    margin-left: 150px;
    padding-top: 20px;
    padding-bottom: 20px;
    border-width: 2px;
    border-style: solid;
    border-color: #DDDDDD;
    background-color: #F5F5F5
  }

  .text {
    font-size: 60px;
    color: #666666;
    text-align: center;
  }
</style>