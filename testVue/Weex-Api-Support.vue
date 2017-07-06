<template>
  <div class="wrapper">
    <div v-for="(key, i) in conditions" class="line">
      <text class="key">{{key}}</text>
      <text :class="['value', results[i] ? 'green' : 'red']">{{results[i]}}</text>
    </div>
  </div>
</template>

<script>
    export default {
        data() {
            return {
                results: [],
                conditions: [
                    '@component/div',
                    '@component/slider',
                    '@component/countdown',

                    '@module/stream',
                    '@module/mtop',
                    '@module/dom',
                    '@module/abcd',
                    '@module/abcd.efgh',
                    '@module/picker.pick',
                    '@module/stream.fetch',
                    '@module/stream.abcde'
                ]
            }
        },
        created() {
            this.results = this.conditions.map(key => {
                try {
                    return weex.supports(key)
                } catch (e) {
                    return 'unknown'
                }
            })
        }
    }
</script>

<style scoped>
    .wrapper {
        flex-direction: column;
        justify-content: center;
    }
    
    .line {
        flex-direction: row;
        margin-bottom: 16px;
    }
    
    .key {
        width: 480px;
        font-size: 40px;
        margin-left: 60px;
        color: #888888;
    }
    
    .value {
        font-size: 40px;
        color: #555555;
    }
    
    .green {
        color: green;
    }
    
    .red {
        color: red;
    }
</style>