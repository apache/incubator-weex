<template>
  <div class="weex-container">
    <span v-if="$slots.default">
      <slot></slot>
    </span>
  </div>
</template>
