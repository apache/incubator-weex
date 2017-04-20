<template>
  <div>
    <div ref="foo" style="height:200px;" :style="{ backgroundColor: 'red' }" class="ct"></div>
  </div>
</template>

<style scoped>
.ct {
  width: 200px;
  flex-direction: row;
  transform: translate3d(100px, 100px, 0);
}
</style>
