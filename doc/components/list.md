# &lt;list&gt;

### Summary

A fixed height container which has multiple child components (cells) arranged in one column. If total height of its child components is higher then the height of itself, the whole child components are scrollable and it's high-performanced.

**Notes:** height of `list` must be specified (list's styles must contain `height` or pair of `top`, `bottom` or flex value if outter container's `flex-direction` value is `column`).

### Child Components

`cell` is a available candidate to be used as a child component for `list`.

### Attributes

- `loadmoreoffset`: &lt;number&gt; the offset distance downwards the bottom of the list to trigger the loadmore event, which is triggered when the list scrolls down near enough to the bottom.

Other attributes please check out the [common attributes](../references/common-attrs.md).

-  `orientation`: &lt;string&gt; determine list item layout in column (`vertical`) or row (`horizontal`), default is `vertical`.


- `transform`: &lt;string&gt; define item transformation. See next section for mmore detail.

### Item Transformation
The `tranform` attribute allow you tranlate,rotate,scale and change transparency of item according distance between item and list component's center in the `orientation` direction. Item right in middle has no transformation applied.   

The value is a simple expression, like    
```
transform(value,[...valueN]);[...transformN(value,[...valueN]);]
```   
Very similar to the 2D transform in CSS3.   
For example,    
```
transform="scale(0.5,0.5);translate(100,0)" 
```    
This means the item will scale to 50% of the origin size and translate 100 in x axis when it arrive the edge of list component.   
**The value you set is the transform will apply to the Edge-Item**.   
These transform functions are supported:  
- `scale(x,y)`: scale item, x and y should be a positive float number. 
- `translate(x,y)`: translate item, `x` and `y` shoule be integer numbers. 
- `opacity(n)`: change the transparency of item, `n` must in `[0,1.0]`.
- `rotate(n)`: rotate item, n is integer number.

### Styles
**common styles**: check out [common styles for components](../references/common-style.md)

- support flexbox related styles
- support box model related styles
- support ``position`` related styles
- support ``opacity``, ``background-color`` etc.

### Events

- `loadmore`: if the list scrolls to bottom, this event will be triggered immediately. You can load the next page of items in this event handler.
- `onappear`: if the list's items scroll to visible area, this event will be triggered when list is stopped . You can do something in this event handler.
- `ondisappear`: if the list's items scroll to invisible area, this event will be triggered when list is stopped . You can do something in this event handler.


**common events**: check out the [common events](../references/common-event.md)

- support `click` event. Check out [common events](../references/common-event.md)
- support `appear` / `disappear` event. Check out [common events](../references/common-event.md)

### Example

```
<template>
  <list>
    <cell repeat="{{list}}">
      <text>{{name}}: ${{price}}</text>
    </cell>
  </list>
</template>

<script>
  module.exports = {
    data: {
      list: [
        {name: '...', price: 100},
        {name: '...', price: 500},
        {name: '...', price: 1.5},
        ...
      ]
    }
  }
</script>
```
