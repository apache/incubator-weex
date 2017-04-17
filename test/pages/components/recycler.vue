<template>
  <waterfall class="page" ref="waterfall" test-id="waterfall"
  v-bind:style="{padding:padding}"
  :column-width="columnWidth" :column-count="columnCount" :column-gap="columnGap"
  :show-scrollbar="showScrollbar" :scrollable="scrollable"
  @scroll="recylerScroll" @loadmore="loadmore" loadmoreoffset=3000
  >
  <!--<refresh class="refresh" @refresh="onrefresh" @pullingdown="onpullingdown" :display="refreshing ? 'show' : 'hide'">
      <loading-indicator class="indicator"></loading-indicator>
      <text class="refreshText">{{refreshText}}</text>
  </refresh>-->
    <header class="header" ref="header" test-id="header1" v-if="showHeader">
      <div class="banner">
       <image class="absolute" src="https://gw.alicdn.com/tps/TB1ESN1PFXXXXX1apXXXXXXXXXX-1000-600.jpg" resize="cover"></image>
       <div class="bannerInfo">
          <image class="avatar" src="https://gw.alicdn.com/tps/TB1EP9bPFXXXXbpXVXXXXXXXXXX-150-110.jpg" resize="cover"></image>
          <text class="name">Adam Cat</text>
          <div class="titleWrap">
            <text class="title">Genius</text>
          </div>
        </div>
        <div class="bannerPhotoWrap">
          <image class="bannerPhoto" v-for="photo in banner.photos" :src="photo.src"></image>
        </div>
      </div>
    </header>
    <header  class="stickyHeader" >
      <div class="header2" test-id="header2">
      <div v-if="stickyHeaderType === 'none'" class="stickyWrapper">
        <text class="stickyText" test-id="stickyText1">Sticky Header</text>
      </div>
      <div v-if="stickyHeaderType === 'appear'" class="stickyWrapper">
        <div class="stickyTextImageWrapper">
          <text class="stickyText">Last Appear:</text>
          <image class="stickyImage" :src="appearImage"></image>
        </div>
        <div class="stickyTextImageWrapper">
          <text class="stickyText">Last Disappear:</text>
          <image class="stickyImage" :src="disappearImage"></image>
        </div>
      </div>
     
      <div v-if="stickyHeaderType === 'scroll'" class="stickyWrapper">
        <text class="stickyText">Content Offset:{{contentOffset}}</text>
      </div>
     </div>
    </header>
  <cell v-for="(item, index) in items" :key="item.src" class="cell" :test-id="'cell' + index" ref="index">
      <div class="item" @click="onItemclick(item.behaviour, index)" @appear="itemAppear(item.src)" @disappear="itemDisappear(item.src)">
        <text v-if="item.name" class="itemName">{{item.name}}</text>
        <image class="itemPhoto" :src="item.src"></image>
        <text v-if="item.desc" class="itemDesc">{{item.desc}}</text>
        <text v-if="item.behaviourName" class="itemClickBehaviour"> {{item.behaviourName}}</text>
      </div>
    </cell>
    <header>
      <div  class="footer" ref="footer" test-id="footer1">
      <text class="stickyText">Footer</text>
      </div>
    </header>
    <div ref="fixed" class="fixedItem" test-id="fixed1" @click="scrollToNext">
      <text class="fixedText">bot</text>
    </div>
  </waterfall>
</template>

<style>
  .page {
    background-color: #EFEFEF;
  }
  .refresh {
    height: 128;
    width: 750;
    flex-direction: row;
    align-items: center;
    justify-content: center;
  }
  .refreshText {
    color: #888888;
    font-weight: bold;
  }
  .indicator {
    color: #888888;
    height: 40;
    width: 40;
    margin-right: 30;
  }
  .absolute {
  position: absolute;
  top: 0px;
  width: 750;
  height: 377;
}
  .banner {
    height: 377;
    flex-direction: row;
  }
  .bannerInfo {
    width:270;
    align-items: center;
    justify-content: center;
  }
  .avatar {
    width: 148;
    height: 108;
    border-radius: 54;
    border-width: 4;
    border-color: #FFFFFF;
    margin-bottom: 14;
  }
  .name {
    font-weight: bold;
    font-size:32;
    color:#ffffff;
    line-height:32;
    text-align:center;
    margin-bottom: 16;
  }
  .titleWrap {
    width: 100;
    height: 24;
    margin-bottom: 10;
    background-color: rgba(255,255,255,0.80);
    border-radius: 12;
    justify-content: center;
    align-items: center;
  }
  .title {
    font-size: 20;
    color: #000000;
  }
  .bannerPhotoWrap {
    width: 449;
    height: 305;
    background-color: #FFFFFF;
    border-radius: 12;
    margin-top: 35;
    padding: 12;
    flex-direction: row;
    justify-content: space-between;
    flex-wrap:wrap;
  }
  .bannerPhoto {
    width: 137;
    height: 137;
    margin-bottom: 6;
  }
  .stickyHeader {
    position: sticky;
    /*height: 94;
    flex-direction: row;
    padding-bottom:6;*/
  }
  .header2 {
    position: sticky;
    height: 94;
    flex-direction: row;
    padding-bottom:6;
  }
  .stickyWrapper {
    flex-direction: row;
    background-color:#00cc99;
    justify-content: center;
    align-items: center;
    flex:1;
  }
  .stickyTextImageWrapper {
    flex:1;
    justify-content: center;
    align-items: center;
    flex-direction: row;
  }
  .stickyText {
    color: #FFFFFF;
    font-weight: bold;
    font-size:32;
    margin-right: 12;
  }
  .stickyImage {
    width: 64;
    height: 64;
    border-radius: 32;
  }

  .cell {
    padding-top: 6;
    padding-bottom: 6;
  }
  .item {
    background-color: #FFFFFF;
    align-items: center;
  }
  .itemName {
    font-size:28;
    color:#333333;
    line-height:42;
    text-align:left;
    margin-top: 24;
  }
  .itemPhoto {
    margin-top: 18;
    width: 220;
    height: 220;
    margin-bottom: 18;
  }
  .itemDesc {
    font-size:24;
    margin:12;
    color:#999999;
    line-height:36;
    text-align:left;
  }
  .itemClickBehaviour {
    font-size:36;
    color:#00cc99;
    line-height:36;
    text-align:center;
    margin-top: 6;
    margin-left: 24;
    margin-right: 24;
    margin-bottom: 30;
  }
  .footer {
    height: 94;
    justify-content: center;
    align-items: center;
    background-color: #00cc99;
  }

  .fixedItem {
    position: fixed;
    width:78;
    height:78;
    background-color:#00cc99;
    right: 32;
    bottom: 32;
    border-radius: 39;
    align-items: center;
    justify-content: center;
  }
  .fixedText {
    font-size: 32;
    color: white;
    line-height: 32;
  }

</style>

<script>
  export default {
    data: function() {
      const items = [
        {
          src:'https://gw.alicdn.com/tps/TB1Jl1CPFXXXXcJXXXXXXXXXXXX-370-370.jpg',
          name: 'Thomas Carlyle',
          desc:'Genius only means hard-working all one\'s life',
          behaviourName: 'Change width',
          behaviour: 'changeColumnWidth'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1Hv1JPFXXXXa3XXXXXXXXXXXX-370-370.jpg',
          desc:'The man who has made up his mind to win will never say "impossible "',
          behaviourName: 'Change gap',
          behaviour: 'changeColumnGap'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1eNKuPFXXXXc_XpXXXXXXXXXX-370-370.jpg',
          desc:'There is no such thing as a great talent without great will - power',
          behaviourName: 'Change count',
          behaviour: 'changeColumnCount'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1DCh8PFXXXXX7aXXXXXXXXXXX-370-370.jpg',
          name:'Addison',
          desc:'Cease to struggle and you cease to live',
          behaviourName: 'Show scrollbar',
          behaviour: 'showScrollbar'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1ACygPFXXXXXwXVXXXXXXXXXX-370-370.jpg',
          desc:'A strong man will struggle with the storms of fate',
          behaviourName: 'Listen appear',
          behaviour: 'listenAppear'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1IGShPFXXXXaqXVXXXXXXXXXX-370-370.jpg',
          name:'Ruskin',
          desc:'Living without an aim is like sailing without a compass',
          behaviourName: 'Set scrollable',
          behaviour: 'setScrollable',
        },
        {
          src:'https://gw.alicdn.com/tps/TB1xU93PFXXXXXHaXXXXXXXXXXX-240-240.jpg',
          behaviourName: 'waterfall padding',
          behaviour: 'setPadding'
        },
        {
          src:'https://gw.alicdn.com/tps/TB19hu0PFXXXXaXaXXXXXXXXXXX-240-240.jpg',
          name:'Balzac',
          desc:'There is no such thing as a great talent without great will - power',
          behaviourName: 'listen scroll',
          behaviour: 'listenScroll'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1ux2vPFXXXXbkXXXXXXXXXXXX-240-240.jpg',
          behaviourName: 'Remove cell',
          behaviour: 'removeCell'
        },
        {
          src:'https://gw.alicdn.com/tps/TB1tCCWPFXXXXa7aXXXXXXXXXXX-240-240.jpg',
          behaviourName: 'Move cell',
          behaviour: 'moveCell'
       
        }
      ]

      let repeatItems = [];
      for (let i = 0; i < 3; i++) {
        repeatItems.push(...items)
      }

      return {
        padding: 0,
        refreshing: false,
        refreshText: '↓   pull to refresh...',
        columnCount: 2,
        columnGap: 12,
        columnWidth: 'auto',
        contentOffset: '0',
        showHeader: true,
        showScrollbar: false,
        scrollable: true,
        showStickyHeader: false,
        appearImage: null,
        disappearImage: null,
        stickyHeaderType: 'none',
        // fixedRect:'',
        banner: {
          photos: [
            {src:'https://gw.alicdn.com/tps/TB1JyaCPFXXXXc9XXXXXXXXXXXX-140-140.jpg'},
            {src:'https://gw.alicdn.com/tps/TB1MwSFPFXXXXbdXXXXXXXXXXXX-140-140.jpg'},
            {src:'https://gw.alicdn.com/tps/TB1U8avPFXXXXaDXpXXXXXXXXXX-140-140.jpg'},
            {src:'https://gw.alicdn.com/tps/TB17Xh8PFXXXXbkaXXXXXXXXXXX-140-140.jpg'},
            {src:'https://gw.alicdn.com/tps/TB1cTmLPFXXXXXRXXXXXXXXXXXX-140-140.jpg'},
            {src:'https://gw.alicdn.com/tps/TB1oCefPFXXXXbVXVXXXXXXXXXX-140-140.jpg'}
          ]
        },
        items: repeatItems
      }
    },

    created() {
      // let self = this
      // setTimeout(()=>{
      //   weex.requireModule('dom').getComponentRect(this.$refs.fixed, result=>{
      //     const x = result.size.left
      //     const y = result.size.top
      //     const width = result.size.width
      //     const height = result.size.height
      //     self.fixedRect = `${x}|${y}|${width}|${height}`
      //   })
      // }, 3000)
    },

    methods: {
      recylerScroll: function(e) {
        this.contentOffset = e.contentOffset.y
      },
      loadmore: function(e) {
        console.log('receive loadmore event')
        // this.$refs.waterfall.resetLoadmore()
      },
      showOrRemoveHeader: function() {
        this.showHeader = !this.showHeader
      },
      onItemclick: function (behaviour, index) {
        console.log(`click...${behaviour} at index ${index}`)
        switch (behaviour) {
          case 'changeColumnCount':
            this.changeColumnCount()
            break
          case 'changeColumnGap':
            this.changeColumnGap()
            break
          case 'changeColumnWidth':
            this.changeColumnWidth()
            break
          case 'showScrollbar':
            this.showOrHideScrollbar()
            break
          case 'listenAppear':
            this.listenAppearAndDisappear()
            break
          case 'setScrollable':
            this.setScrollable()
            break
          case 'setPadding':
            this.setRecyclerPadding()
            break
          case 'listenScroll':
            this.listenScrollEvent()
            break
          case 'removeCell':
            this.removeCell(index)
            break
          case 'moveCell':
            this.moveCell(index)
            break
        }
      },

      itemAppear: function(src) {
        this.appearImage = src;
      },

      itemDisappear: function(src) {
        this.disappearImage = src;
      },

      changeColumnCount: function() {
        if (this.columnCount === 2) {
          this.columnCount = 3
        } else {
          this.columnCount = 2
        }
      },

      changeColumnGap: function() {
        if (this.columnGap === 12) {
          this.columnGap = 'normal'
        } else {
          this.columnGap = 12
        }
      },

      changeColumnWidth: function() {
        if (this.columnWidth === 'auto') {
          this.columnWidth = 600
        } else {
          this.columnWidth = 'auto'
        }
      },

      showOrHideScrollbar: function() {
        this.showScrollbar = !this.showScrollbar
      },

      setScrollable: function() {
        this.scrollable = !this.scrollable
      },

      listenAppearAndDisappear: function() {
        this.stickyHeaderType = (this.stickyHeaderType === 'appear' ? 'none' : 'appear')
      },

      listenScrollEvent: function() {
        this.stickyHeaderType = (this.stickyHeaderType === 'scroll' ? 'none' : 'scroll')
      },

      scrollToNext: function() {
        weex.requireModule('dom').scrollToElement(this.$refs.footer,{})
      },

      setRecyclerPadding: function() {
        this.padding = (this.padding == 0 ? 12 : 0);
      },

      removeCell: function(index) {
        this.items.splice(index, 1)
      },

      moveCell: function(index) {
        if (index == 0) {
          this.items.splice(this.items.length - 1, 0, this.items.splice(index, 1)[0]);
        } else {
          this.items.splice(0, 0, this.items.splice(index, 1)[0]);
        }
      },

      onrefresh (event) {
        this.refreshing = true
        this.refreshText = "loading..."
        setTimeout(() => {
          this.refreshing = false
          this.refreshText = '↓   pull to refresh...'
        }, 2000)
      },

      onpullingdown (event) {
        // console.log(`${event.pullingDistance}`)
        if (event.pullingDistance < -64) {
          this.refreshText = '↑   release to refresh...'
        } else {
          this.refreshText = '↓   pull to refresh...'
        }
      }
    }
  }
</script>
