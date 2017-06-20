<style>
  .container {
    flex: 1;
    justify-content: center;
    align-items: center;
    background-color: #f8f8f8;
    flex-direction: column;
    border-top-style: solid;
    border-top-width: 2;
    border-top-color: #dfdfdf;
  }

  .list {
    flex: 1;
    width: 750;
    /*transform:rotate(180deg);*/
    justify-content: center;
    align-items: center;
    background-color: #abcdef;
    flex-direction: column;
    border-top-style: solid;
    border-top-width: 2;
    border-top-color: #dfdfdf;
  }

  .row {

  }

  .item {
    justify-content: center;
    border-bottom-width: 2px;
    border-bottom-color: #c0c0c0;
    width:750px;
    height: 100px;
    padding: 20px;
    /*transform:rotate(180deg);*/
  }

  .item-title {
    font-size: 30px;
  }
  .cell{

  }
  .header {
    background-color: #f00;
    width:750px;
    height: 60px;
    justify-content: center;
  }
</style>

<template>
  <div class="container">
    <list class="list" loadmore="loadmore" loadmoreoffset="500" >
      <refresh ref="refresh" class="refresh" @refresh="insertData"
               :display="isPullingDown ? 'show' : 'hide'">
        <div class="refresh-text">
          <text class="more-text" v-if="!isPullingDown">下拉加载更多</text>
          <text class="more-text" v-if="isPullingDown">加载中...</text>
        </div>
      </refresh>
      <template v-for="row in rows">
        <header class="header" keepScrollPosition="true"  :key="row.id">
          <div class = "header">
          <text>{{row.id}}{{row.headerText}}</text>
          </div>
        </header>

        <cell class="cell"  v-for="item in row.item"  keepScrollPosition="true" append="tree" :key="item.id">
          <div class="item">
            <text class="item-title">{{item.cellText}}</text>
          </div>
        </cell>
      </template>
      <cell @click="onFooterClick" style="height: 90; background-color: #396">
        <text style="font-size: 30; color: #fff;">footer</text>
      </cell>
    </list>
  </div>
</template>


<script>
  let guid = 0;
  module.exports = {
    created: function() {
      for (var i = 0; i < 30; i++) {
        var data = {
          id: i,
          headerText: '原有标题',
          item: [
            {
              id: i * 10,
              cellText: i +'原有内容',
            },
            {
              id: i * 10,
              cellText: i +'原有内容',
            }
          ]
        };
        this.rows.push(data);

      }
      console.log(this.$getConfig().env)
    },
    methods: {
      onappear: function(e) {
        var appearId = this.rows[e.target.attr.index].id;
        nativeLog('+++++', appearId);
        var appearIds = this.appearIds;
        appearIds.push(appearId);
        this.getMinAndMaxIds(appearIds);
      },
      ondisappear: function(e) {
        var disAppearId = this.rows[e.target.attr.index].id;
        nativeLog('+++++', disAppearId);
        var appearIds = this.appearIds;
        var index = appearIds.indexOf(disAppearId);
        if (index > -1) {
          appearIds.splice(index, 1);
        }
        this.getMinAndMaxIds(appearIds);
      },
      getMinAndMaxIds: function(appearIds) {
        appearIds.sort(function(a, b) {
          return a - b;
        });
        this.appearIds = appearIds;
        this.appearMax = appearIds[appearIds.length - 1];
        this.appearMin = appearIds[0];
      },
      myRefresh: function(){
        console.log("into--[onrefresh]");
        this.isPullingDown=true;
      },
      
      creatTestData : function(){
        return {
          id: guid++,
          headerText: ('插入的标题' + guid++),
          item: [
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
           {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
           {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
          ]
        };        
      }, 
      
      insertData: function(e) {
        this.isPullingDown=true
        var _this = this
        setTimeout(function() {
          setTimeout(function(){
            _this.rows.unshift(_this.creatTestData());
          }, 1000);
          _this.isPullingDown = false;
        }, 1000)
      },
      
      
      onFooterClick : function(){
        this.rows.splice(0, 5);
        this.rows.unshift({
          id: guid++,
          headerText: ('插入的标题' + guid++),
          item: [
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
           {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
           {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
            {
              id: guid++,
              cellText: '插入的内容'+parseInt(Math.random() * 700 + 1000),
            },
          ]
        })
      }
    },
    data: {
      appearMin: 1,
      appearMax: 1,
      appearIds: [],
      rows: [],
      position: 'normal',
      isPullingDown: false
    }
  }
</script>
