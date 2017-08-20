---
title: <waterfall>
type: references
order: 2.5
version: 2.1
---

# waterfall

<span class="weex-version">v0.11.0+</span>

提供瀑布流布局的组件

### 子组件

注意:  和list一样, waterfall 只支持特定类型的组件: cell, header, refresh, loading 和 fixed-position 组件.

- `cell`: 瀑布流中的每个元素
- `header`: 主要用于表示横跨多列的元素，可以通过css的position属性设置为sticky

### 特性

- **column-width** : 描述瀑布流每一列的列宽
  - `auto`: 意味着列宽是被其他属性所决定的(比如 column-count)
  - `<length>`: 最佳列宽，实际的列宽可能会更宽(需要填充剩余的空间)， 或者更窄(如果剩余空间比列宽还要小)。 该值必须大于0
- **column-count**: 描述瀑布流的列数
  - `auto`: 意味着列数是被其他属性所决定的(比如 column-width)
  - `<integer>`: 最佳列数，column-width 和  column-count 都指定非0值， 则 column-count 代表最大列数。
- **column-gap**: 列与列的间隙. 如果指定了 `normal` ，则对应 `32`.

其他支持的属性参见 [List Component Attributes](./list.html)

### 样式

通用样式：支持所有通用样式

- 盒模型
- `flexbox` 布局
- `position`
- `opacity`
- `background-color`

查看 [组件通用样式](../common-style.html)

### 事件

- 通用事件

  支持所有通用事件：

  - `click`
  - `longpress`
  - `appear`
  - `disappear`

  查看 [通用事件](../common-event.html)

### API

滚动到列表某个指定项是常见需求，`<waterfall>` 拓展了该功能支持滚动到指定 `<cell>` 或者 `<header>`。通过 `dom` module 访问，更多信息可参考 [dom module](../modules/dom.html) 。

### 示例

参见 [playground waterfall example](http://dotwe.org/vue/7a9195643e9e8da352b0d879cdbe68c0)