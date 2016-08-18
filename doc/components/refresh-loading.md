# &lt;refresh-loading&gt;

### Summary

* `refresh` component can be used inside list/scroller to add pull to refresh functionality, the style of headerview can be set by specifying the style of refresh.
* `loading` component can be used inside list/scroller to add loading view at the bottom of container, the style of footerview can be set by specifying the style of loading.

### Child Components

**Notes:** `refresh` and `loading` is able to contain any type of sub-components you want, default loading animation can be achieved by using `loading-indicator` component from weex.

* `loading-indicator` default loading animation can be achieved by using this component in refresh and loading component，supports `color` property; Default `width` and `height` of `loading-indicator` is 0, so it must specify its `width` and `height`.

### Attributes

- `display` is a switch which toggles the visibility of refresh and loading component. `display` should be set to `show` in callback of `onrefresh` and `onloading`, and set to `hide` after refresh and loading is done.

Other attributes please check out the [common attributes](../references/common-attrs.md).

### Events

- support `onrefresh` event  <sup class="wx-v">0.5</sup>.  If a `refresh` component is inside `scroller` / `list`, this event will be triggered when it is pull down refresh.
- support `onloading` event .  If a `loading` component is inside `scroller` / `list`, this event will be triggered when it is pull up loadmore.

### Example

``` javaScript

<template>
  <div>
    <list class="list">
      <refresh  class = "refresh-view" display="{{refresh_display}}" onrefresh="onrefresh">
        <text class="refresh-arrow" if="{{(refresh_display==='hide')}}"> ↓ Pull Refresh </text>
        <loading-indicator class="indicator"></loading-indicator>
      </refresh>

      <cell class="row" repeat="{{rows}}" index="{{$index}}">
        <div class="item">
          <text class="item-title">row {{id}}</text>
        </div>
      </cell>

      <loading class="loading-view" display="{{loading_display}}" onloading="onloading">
        <text class="refresh-arrow" if="{{(loading_display==='hide')}}"> ↑ Loadmore </text>
        <loading-indicator class="indicator"></loading-indicator>
      </loading>
    </list>
  </div>
</template>

<style>
  .list {
    flex: 1;
  }
  .row {
    width: 750px;
  }
  .item {
    justify-content: center;
    border-bottom-width: 2px;
    border-bottom-color: #c0c0c0;
    height: 100px;
    padding:20px;
  }
  .item-title {
  }
  .refresh-view {
    width: 750;
    height: 100;
    display: -ms-flex;
    display: -webkit-flex;
    display: flex;
    -ms-flex-align: center;
    -webkit-align-items: center;
    -webkit-box-align: center;
    align-items: center;
  }
  .loading-view {
    width: 750;
    height: 100;
    display: -ms-flex;
    display: -webkit-flex;
    display: flex;
    -ms-flex-align: center;
    -webkit-align-items: center;
    -webkit-box-align: center;
    align-items: center;
  }
  .indicator {
    height: 60;
    width: 60;
    color: blue;
  }
</style>

<script>
  require('weex-components');
  module.exports = {
    methods: {
      onrefresh: function(e) {
        var self = this;
        self.refresh_display = 'show';
        self.$call('modal', 'toast', {
          'message': 'onrefresh'
        });

        this.$call('timer', 'setTimeout', function () {
          self.refresh_display = 'hide';
        }, 3000);
      },

      onloading: function() {
        var self = this;
        self.loading_display = 'show';
        self.$call('modal', 'toast', {
          'message': 'onloading'
        });

        this.$call('timer', 'setTimeout', function () {
          self.loading_display = 'hide';
        }, 3000);
      },
    },
    data: {
      rows:[
        {id: 1},{id: 2},{id: 3},{id: 4},{id: 5},{id: 6},
        {id: 7},{id: 8},{id: 9},{id: 10},{id: 11},{id: 12},
        {id: 13},{id: 14},{id: 15},{id: 16},{id: 17},{id: 18},
        {id: 19},{id: 20},{id: 21},{id: 22},{id: 23},{id: 24},
        {id: 25},{id: 26},{id: 27},{id: 28},{id: 29}
      ]
    }
  }
</script>

```
