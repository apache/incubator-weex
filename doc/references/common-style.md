# Common Style

All of weex tags share some common style rules

## Box Model

![box model](http://www.codeproject.com/KB/HTML/567385/boxmodel-image.png)

Weex box model based on the CSS box model, all of weex elements can be considered as boxes.  The term "box model" is used when talking about design and layout. The box model is essentially a box that wraps around every HTML element. It consists of margins, borders, paddings, and the actual content.

you can use the definition below in weex box model.

- width
- height
- padding (space around content, between element content and the element border)
  - padding-left
  - padding-right
  - padding-top
  - padding-bottom
- margin (space around elements, outside the border)
  - margin-left
  - margin-right
  - margin-top
  - margin-bottom
- border
  - border-style(solid,dashed,dotted)
  - border-width
    - border-left-width
    - border-top-width
    - border-right-width
    - border-bottom-width
  - border-color
    - border-left-color
    - border-top-color
    - border-right-color
    - border-bottom-color
  - border-radius  ( rounded borders to elements , default value is 0 meaning right angle )
    - border-bottom-left-radius
    - border-bottom-right-radius
    - border-top-left-radius
    - border-top-right-radius

Notes: The rule of border-radius for a specific corner such as `border-top-left-radius` is not currently supported for component `<image>` and `<text>`.

Weex box model uses `border-box` as the default value of `box-sizing`, meaning the width and height properties includes content, padding and border, but not the margin.

example:

```html
<template>
  <div>
    <image src="..." style="width: 400; height: 200; margin-left: 20;"></image>
  </div>
</template>
```

## Flexbox

Weex box style model based on the CSS flexbox, ensures that elements behave predictably and the page layout can accommodates to different screen sizes and different display devices.

Flexbox consists of flex containers and flex items. If a weex element can containing other elements, it is a flex container.

Notice that the old version of flexbox specification has differences with the new ones, such as whether or not to support wrapping. This is described at w3c's working drafts, and you should notice the differences among them. Also notice that the old version is only supported below the 4.4 version of android.

### Flex container

Flexbox is the default and only style model in Weex, so you don't have to add `display: flex;` in a container.

- `flex-direction`: `row` | `column`

The flex-direction property specifies the direction of the flexible items inside the flex container. The default value of flex-direction is `row` (left-to-right, top-to-bottom).

- `justify-content`: `flex-start` | `flex-end` | `center` | `space-between`

The justify-content property horizontally aligns the flexible container's items when the items do not use all available space on the main-axis. `flex-start` is the default value, meaning the flex items are positioned at the beginning of the container. `flex-end`` means the items are positioned at the end of the container. `center` means the items are positioned at the center of the container. `space-between` means the items are positioned with space between the lines.

![justify-content](http://www.w3.org/TR/css3-flexbox/images/flex-pack.svg)

- `align-items`: `stretch` | `flex-start` | `center` | `flex-end`

The align-items property vertically aligns the flexible container's items when the items do not use all available space on the cross-axis. `stretch` is  the default value, meaning the items are stretched to fit the container. `flex-start` means the items are positioned at the top of the container; `flex-end` means the items are positioned at the bottom of the container; `center` means items are positioned at the center of the container (vertically).

![align-items](http://gtms02.alicdn.com/tps/i2/TB1VnHKMXXXXXcEaXXXDldN_pXX-1018-502.jpg)

### Flex item

- flex : &lt;number&gt;

the flex property specifies the length of the flex item, relative to the rest of the flex items inside the same container.  If all of the flex items set `flex: 1`, they will have equal width or height on direction of flex container's `flex-direction`. If there are two flex items, with one setting `flex: 1`, and the other setting `flex: 2`, the first one will take 1/3 container space, and the second one will take 2/3 container space. If all of flex items don't set `flex`, they will be aligned depending on the container's `justify-content` property.


## Examples

a list of images with equal scales align at the vertical axis:

```html
<template>
  <div style="width: 300; height: 100;">
    <image src="..." style="flex: 1;"></image>
    <image src="..." style="flex: 1;"></image>
    <image src="..." style="flex: 1;"></image>
  </div>
</template>
```

a image with fixed width aligns with a stretched text:

```html
<template>
  <div style="width: 300; height: 100;">
    <image src="..." style="width: 100; height: 100;"></image>
    <text style="flex: 1;">...</text>
  </div>
</template>
```

mixed direction alignment:

```html
<template>
  <div style="width: 100;">
    <image src="..." style="width: 100; height: 100;"></image>
    <div style="flex-direction: row;">
      <text style="flex: 2; font-size: 32;">title</text>
      <text style="flex: 1; font-size: 16;">$100</text>
    </div>
  </div>
</template>
```

## Position

we can use properties below to control placement of weex tag

- `position`: `relative` | `absolute` | `fixed` | `sticky`, `relative` is the default value

`relative` means the item is positioned relative to its normal position. `absolute` means the item is positioned relative to its container. `fixed` keeps the elements position fixed when the page is scrolling. `sticky` keeps elements positioned inside the viewport as "stuck" at the top or "relative" at its original place depending on whether does it about to scroll out of the view.

- `top`:  &lt;number&gt;, upward offset value, default `0`
- `bottom`: &lt;number&gt;, downward offset value, default `0`
- `left`: &lt;number&gt;, leftward offset value, default `0`
- `right`: &lt;number&gt;, rightward offset value, default `0`

### Examples

```html
<template>
  <div style="flex-direction: column;">
    <div style="height: 3000;">
      <image src="..." style="top: 50; left: 50; ..."></image>
    </div>
    <div style="height: 3000;">
      <image src="..." style="position: sticky; ..."></image>
    </div>
    <div style="height: 3000;">
      <image src="..." style="position: absolute; top: 50; left: 50; ..."></image>
    </div>
  </div>
</template>
```

## Other Common Style

- `opacity`:  &lt;number&gt;, value within range 0 to 1. 1 is not transparent at all, 0.5 is 50% see-through, and 0 is completely transparent. 1 is the default value.
- `background-color`: &lt;colors&gt; sets the background color of an element, `transparent` is the default value.

## Type of Style Value

- length: number followed by length unit `px`, `px` can be omitted.
- colors: support multiple formats of values, including rgb (`rgb(255, 0, 0)`), rgba (`rgba(255, 0, 0, 0.5)`), hexadecimal (`#ff0000`), short hexadecimal (`#f00`), named color (`red`).
- enumerated values: a limited number of string values.

**Note: The list of color keywords:**

Basic color keywords:

| Color Name | Hex rgb |
| ---------- | ------- |
| black | #000000 |
| silver |  #C0C0C0 |
| gray |  #808080 |
| white | #FFFFFF |
| maroon |  #800000 |
| red | #FF0000 |
| purple |  #800080 |
| fuchsia | #FF00FF |
| green | #008000 |
| lime |  #00FF00 |
| olive | #808000 |
| yellow |  #FFFF00 |
| navy |  #000080 |
| blue |  #0000FF |
| teal |  #008080 |
| aqua |  #00FFFF |

Extended color keywords:

| Color Name | Hex rgb |
| ---------- | ------- |
| aliceblue | #F0F8FF |
| antiquewhite |  #FAEBD7 |
| aqua |  #00FFFF |
| aquamarine |  #7FFFD4 |
| azure | #F0FFFF |
| beige | #F5F5DC |
| bisque |  #FFE4C4 |
| black | #000000 |
| blanchedalmond |  #FFEBCD |
| blue |  #0000FF |
| blueviolet |  #8A2BE2 |
| brown | #A52A2A |
| burlywood | #DEB887 |
| cadetblue | #5F9EA0 |
| chartreuse |  #7FFF00 |
| chocolate | #D2691E |
| coral | #FF7F50 |
| cornflowerblue |  #6495ED |
| cornsilk |  #FFF8DC |
| crimson | #DC143C |
| cyan |  #00FFFF |
| darkblue |  #00008B |
| darkcyan |  #008B8B |
| darkgoldenrod | #B8860B |
| darkgray |  #A9A9A9 |
| darkgreen | #006400 |
| darkgrey |  #A9A9A9 |
| darkkhaki | #BDB76B |
| darkmagenta | #8B008B |
| darkolivegreen |  #556B2F |
| darkorange |  #FF8C00 |
| darkorchid |  #9932CC |
| darkred | #8B0000 |
| darksalmon |  #E9967A |
| darkseagreen |  #8FBC8F |
| darkslateblue | #483D8B |
| darkslategray | #2F4F4F |
| darkslategrey | #2F4F4F |
| darkturquoise | #00CED1 |
| darkviolet |  #9400D3 |
| deeppink |  #FF1493 |
| deepskyblue | #00BFFF |
| dimgray | #696969 |
| dimgrey | #696969 |
| dodgerblue |  #1E90FF |
| firebrick | #B22222 |
| floralwhite | #FFFAF0 |
| forestgreen | #228B22 |
| fuchsia | #FF00FF |
| gainsboro | #DCDCDC |
| ghostwhite |  #F8F8FF |
| gold |  #FFD700 |
| goldenrod | #DAA520 |
| gray |  #808080 |
| green | #008000 |
| greenyellow | #ADFF2F |
| grey |  #808080 |
| honeydew |  #F0FFF0 |
| hotpink | #FF69B4 |
| indianred | #CD5C5C |
| indigo |  #4B0082 |
| ivory | #FFFFF0 |
| khaki | #F0E68C |
| lavender |  #E6E6FA |
| lavenderblush | #FFF0F5 |
| lawngreen | #7CFC00 |
| lemonchiffon |  #FFFACD |
| lightblue | #ADD8E6 |
| lightcoral |  #F08080 |
| lightcyan | #E0FFFF |
| lightgoldenrodyellow |  #FAFAD2 |
| lightgray | #D3D3D3 |
| lightgreen |  #90EE90 |
| lightgrey | #D3D3D3 |
| lightpink | #FFB6C1 |
| lightsalmon | #FFA07A |
| lightseagreen | #20B2AA |
| lightskyblue |  #87CEFA |
| lightslategray |  #778899 |
| lightslategrey |  #778899 |
| lightsteelblue |  #B0C4DE |
| lightyellow | #FFFFE0 |
| lime |  #00FF00 |
| limegreen | #32CD32 |
| linen | #FAF0E6 |
| magenta | #FF00FF |
| maroon |  #800000 |
| mediumaquamarine |  #66CDAA |
| mediumblue |  #0000CD |
| mediumorchid |  #BA55D3 |
| mediumpurple |  #9370DB |
| mediumseagreen |  #3CB371 |
| mediumslateblue | #7B68EE |
| mediumspringgreen | #00FA9A |
| mediumturquoise | #48D1CC |
| mediumvioletred | #C71585 |
| midnightblue |  #191970 |
| mintcream | #F5FFFA |
| mistyrose | #FFE4E1 |
| moccasin |  #FFE4B5 |
| navajowhite | #FFDEAD |
| navy |  #000080 |
| oldlace | #FDF5E6 |
| olive | #808000 |
| olivedrab | #6B8E23 |
| orange |  #FFA500 |
| orangered | #FF4500 |
| orchid |  #DA70D6 |
| palegoldenrod | #EEE8AA |
| palegreen | #98FB98 |
| paleturquoise | #AFEEEE |
| palevioletred | #DB7093 |
| papayawhip |  #FFEFD5 |
| peachpuff | #FFDAB9 |
| peru |  #CD853F |
| pink |  #FFC0CB |
| plum |  #DDA0DD |
| powderblue |  #B0E0E6 |
| purple |  #800080 |
| red | #FF0000 |
| rosybrown | #BC8F8F |
| royalblue | #4169E1 |
| saddlebrown | #8B4513 |
| salmon |  #FA8072 |
| sandybrown |  #F4A460 |
| seagreen |  #2E8B57 |
| seashell |  #FFF5EE |
| sienna |  #A0522D |
| silver |  #C0C0C0 |
| skyblue | #87CEEB |
| slateblue | #6A5ACD |
| slategray | #708090 |
| slategrey | #708090 |
| snow |  #FFFAFA |
| springgreen | #00FF7F |
| steelblue | #4682B4 |
| tan | #D2B48C |
| teal |  #008080 |
| thistle | #D8BFD8 |
| tomato |  #FF6347 |
| turquoise | #40E0D0 |
| violet |  #EE82EE |
| wheat | #F5DEB3 |
| white | #FFFFFF |
| whitesmoke |  #F5F5F5 |
| yellow |  #FFFF00 |
| yellowgreen | #9ACD32 |

## Simple Step

These up-to-down steps may help you to plan the whole style of weex pages.

1. overall style: divide the whole page to different parts
2. flex alignment: align boxes in every part of page
3. position box: place box, set offset
4. special style: add special styles if needed


