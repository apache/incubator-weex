<template>
  <list class="list">
    <cell class="cell" @click="clickCell">
      <xlink style="width: 400px;height: 400px;background-color: yellow;" :href = "url">
        <div class="ct" @click="click">
          <text >CLICK ME</text>
        </div>
      </xlink>
    </cell>
  </list>
</template>

<style scoped>
.list {
  height: 2000px;
}
.cell {
  height: 400px;
  background-color: blue;
}
.txt {
  font-size: 64px;
  height: 84px;
  line-height: 84px;
  padding: 10px;
}
.ct {
  width: 200px;
  height: 200px;
  background-color: #fff;
}
</style>

<script>
export default {
  data () {
    return {
      // url: '//m.taobao.com'
      url: '#'
    }
  },
  components: {
    xlink: {
      props: {
        href: String
      },
      render (createElement) {
        return createElement('a', {
          ref: 'link',
          attrs: {
            'component-name': 'link',
            'href': this.href
          }
        }, this.$slots.default)
      }
    }
  },
  methods: {
    click (evt) {
      evt.preventDefault()
      // var d = Date.now()
      // while (true) {
      //   var now = Date.now()
      //   if (now - d > 2000) { break }
      // }
      // throw new Error('test')
      console.log('click on inner')
    },
    clickCell (evt) {
      console.log('click on cell')
    }
  }
}
</script>