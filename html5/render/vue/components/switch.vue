<template>
  <span :class="wrapperClass" @click="toggle">
    <small class="weex-switch-inner"></small>
  </sapn>
</template>

<script>
  export default {
    props: {
      checked: {
        type: [Boolean, String],
        default: false
      },
      disabled: {
        type: [Boolean, String],
        default: false
      },
    },
    data () {
      return {
        isChecked: (this.checked !== 'false' && this.checked !== false),
        isDisabled: (this.disabled !== 'false' && this.disabled !== false)
      }
    },
    computed: {
      wrapperClass() {
        return [
          'weex-switch',
          {
            'weex-switch-checked': this.isChecked,
            'weex-switch-disabled': this.isDisabled
          }
        ]
      }
    },
    methods: {
      toggle () {
        // TODO: handle the events
        this.$emit('click', { value: this.isChecked })
        if (!this.isDisabled) {
          this.isChecked = !this.isChecked
          this.$emit('change', { value: this.isChecked })
        }
      }
    },
    mounted () {
      this.$emit('appear', { value: this.isChecked })
    },
    destroyed () {
      this.$emit('disappear', { value: this.isChecked })
    }
  }
</script>
