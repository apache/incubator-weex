# &lt;list&gt;

### Summary

A fixed height container which has multiple child components (cells) arranged in one column. If total height of its child components is higher then the height of itself, the whole child components are scrollable and it's high-performanced.

**Notes:** height of `list` must be specified (list's styles must contain `height` or pair of `top`, `bottom` or flex value if outter container's `flex-direction` value is `column`).

### Child Components

* `cell` is a available candidate to be used as a child component for `list`.

* `refresh` component can be used inside list to add pull to refresh functionality.
* `loading` component can be used inside list to add loading view at the bottom of scroller.

### Attributes

- `loadmoreoffset`: &lt;number&gt; the offset distance downwards the bottom of the list to trigger the loadmore event, which is triggered when the list scrolls down near enough to the bottom.

Other attributes please check out the [common attributes](../references/common-attrs.md).

### Styles

**common styles**: check out [common styles for components](../references/common-style.md)

- support flexbox related styles
- support box model related styles
- support ``position`` related styles
- support ``opacity``, ``background-color`` etc.

### Events

- `loadmore`: if the list scrolls to bottom, this event will be triggered immediately. You can load the next page of items in this event handler.

**common events**: check out the [common events](../references/common-event.md)

- support `click` event. Check out [common events](../references/common-event.md)
- support `appear` / `disappear` event. Check out [common events](../references/common-event.md)
- support `refresh` event .  If a refresh component is inside scroller, this event will be triggered when the scroller is swiping down.
- support `loading` event .  If a loading component is inside scroller, this event will be triggered when user scrolling to the bottom.

### Example

```
<template>
  <div>
    <list class="list">
      <refresh class="refresh-view" display="{{refresh_display}}" onrefresh="onrefresh">
        <text class="refresh-arrow" if="{{(refresh_display==='hide')}}">↓ Pull To Refresh</text>
        <loading-indicator class="indicator"></loading-indicator>
      </refresh>
      <cell onappear="onappear" ondisappear="ondisappear" class="row" repeat="{{rows}}" index="{{$index}}">
        <div class="item">
          <text class="item-title">row {{id}}</text>
        </div>
      </cell>
    </list>
    <text class="count" value="Appear items:{{appearMin}} - {{appearMax}}"></text>
  </div>
</template>

<style>
  .list {
    height: 810px;
    border-width: 10px;
    border-color: orange;
  }
  .count {
    font-size: 48px;
    margin:10px;
  }
  .refresh-view {
    height: 80px;
    width: 750px;
    justify-content: center;
    align-items: center;
  }
  .refresh-arrow {
    font-size: 30px;
    color: #45b5f0;
  }
  .indicator {
    height: 40px;
    width: 40px;
    color:#45b5f0;
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

</style>

<script>
  require('weex-components');
  module.exports = {
    methods: {
      onrefresh: function(e) {
        var self = this;
        self.refresh_display = 'show';
        self.$call('timer', 'setTimeout', function() {
          self.refresh_display = 'hide';
        },3000);
      },
      onappear: function (e) {
        var appearId = this.rows[e.target.attr.index].id;
        var appearIds = this.appearIds;
        appearIds.push(appearId);
        this.getMinAndMaxIds(appearIds);
      },
      ondisappear:function (e) {
        var disAppearId = this.rows[e.target.attr.index].id;
        var appearIds = this.appearIds;
        var index = appearIds.indexOf(disAppearId);
        if (index > -1) {
          appearIds.splice(index, 1);
        }
        this.getMinAndMaxIds(appearIds);
      },
      getMinAndMaxIds:function (appearIds) {
        appearIds.sort(function(a, b) {
          return a - b;
        });
        this.appearIds = appearIds;
        this.appearMax = appearIds[appearIds.length - 1];
        this.appearMin = appearIds[0];
      }
    },
    data: {
      refresh_display : 'hide',
      loading_display : 'hide',
      appearMin:1,
      appearMax:1,
      appearIds:[],
      rows:[
        {id: 1},
        {id: 2},
        {id: 3},
        {id: 4},
        {id: 5},
        {id: 6},
        {id: 7},
        {id: 8},
        {id: 9},
        {id: 10},
        {id: 11},
        {id: 12},
        {id: 13},
        {id: 14},
        {id: 15},
        {id: 16},
        {id: 17},
        {id: 18},
        {id: 19},
        {id: 20},
        {id: 21},
        {id: 22},
        {id: 23},
        {id: 24},
        {id: 25},
        {id: 26},
        {id: 27},
        {id: 28},
        {id: 29}
      ]
    }
  }
</script>
```
