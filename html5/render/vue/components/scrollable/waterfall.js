/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND,  either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * @fileoverview waterfall
 * NOTE: only support full screen width waterfall.
 */

import { scrollable } from './mixins'

const NORMAL_GAP_SIZE = 32
const DEFAULT_COLUMN_COUNT = 1

function getWaterfall (weex) {
  const {
    extractComponentStyle,
    createEventMap
  } = weex

  return {
    name: 'weex-waterfall',
    mixins: [scrollable],
    props: {
      /**
       * specified gap size.
       * value can be number or 'normal'. 'normal' (32px) by default.
       */
      columnGap: {
        type: [String, Number],
        default: 'normal',
        validator (val) {
          if (!val || val === 'normal') {
            return true
          }
          val = parseInt(val)
          return !isNaN(val) && val > 0
        }
      },
      /**
       * the maximum column counts.
       * value can be number or 'auto'. 1 by default.
       */
      columnCount: {
        type: [String, Number],
        default: DEFAULT_COLUMN_COUNT,
        validator (val) {
          val = parseInt(val)
          return !isNaN(val) && val > 0
        }
      },
      /**
       * the mimimum column width.
       * value can be number or 'auto'. 'auto' by default.
       */
      columnWidth: {
        type: [String, Number],
        default: 'auto',
        validator (val) {
          if (!val || val === 'auto') {
            return true
          }
          val = parseInt(val)
          return !isNaN(val) && val > 0
        }
      }
    },

    mounted () {
      this._nextTick()
    },

    updated () {
      this.$nextTick(this._nextTick())
    },

    methods: {
      _createChildren (h, rootStyle) {
        const slots = this.$slots.default || []
        this._headers = []
        this._footers = []
        this._others = []
        const len = slots.length

        for (let i = 0; i < len; i++) {
          const vnode = slots[i]
          const tag = vnode.componentOptions && vnode.componentOptions.tag
          if (tag === 'refresh' || tag === 'loading') {
            continue
          }
          if (tag === 'cell') {
            break
          }
          if (tag === 'header') {
            this._headers.push(vnode)
          }
        }

        for (let i = len - 1; i >= 0; i--) {
          const vnode = slots[i]
          const tag = vnode.componentOptions && vnode.componentOptions.tag
          if (tag === 'refresh' || tag === 'loading') {
            continue
          }
          if (tag === 'cell') {
            break
          }
          if (tag === 'header') {
            this._footers.push(vnode)
          }
        }

        this._cells = slots.filter(vnode => {
          if (!vnode.tag || !vnode.componentOptions) return false
          const tag = vnode.componentOptions.tag
          if (tag === 'refresh' || tag === 'loading') {
            this[`_${tag}`] = vnode
            return false
          }
          if (tag !== 'cell') {
            this._others.push(vnode)
            return false
          }
          return true
        })

        this._reCalc(rootStyle)
        this._genColumns(h)
        let children = []
        this._refresh && children.push(this._refresh)
        children = children.concat(this._headers)
          // .concat(this._others)
        children.push(h('html:div', {
          ref: 'columns',
          staticClass: 'weex-waterfall-inner-columns weex-ct'
        }, this._columns))
        children.push(h('html:div', {
          ref: 'footers',
          staticClass: 'weex-waterfall-footers weex-ct'
        }, this._footers))
        this._loading && children.push(this._loading)
        return [
          h('article', {
            ref: 'inner',
            staticClass: 'weex-waterfall-inner weex-ct'
          }, children)
        ]
      },

      _reCalc (rootStyle) {
        /**
         * NOTE: columnGap and columnWidth can't both be auto.
         * NOTE: the formula:
         *  totalWidth = n * w + (n - 1) * gap
         * 1. if columnCount = n then calc w
         * 2. if columnWidth = w then calc n
         * 3. if columnWidth = w and columnCount = n then calc totalWidth
         *    3.1 if totalWidth < ctWidth then increase columnWidth
         *    3.2 if totalWidth > ctWidth then decrease columnCount
         */
        let width, gap, cnt, ctWidth
        const scale = weex.config.env.scale
        const el = this.$el
        function getCtWidth (width, style) {
          const padding = style.padding
            ? parseInt(style.padding) * 2
            : parseInt(style.paddingLeft || 0) + parseInt(style.paddingRight || 0)
          return width - padding
        }
        if (el && el.nodeType === 1) {  // already mounted
          const cstyle = window.getComputedStyle(el)
          ctWidth = getCtWidth(el.getBoundingClientRect().width, cstyle)
        }
        else {  // not mounted.
          // only support full screen width for waterfall component.
          ctWidth = getCtWidth(document.documentElement.clientWidth, rootStyle)
        }

        gap = this.columnGap
        if (gap && gap !== 'normal') {
          gap = parseInt(gap)
        }
        else {
          gap = NORMAL_GAP_SIZE
        }
        gap = gap * scale

        width = this.columnWidth
        cnt = this.columnCount
        if (width && width !== 'auto') {
          width = parseInt(width) * scale
        }
        if (cnt && cnt !== 'auto') {
          cnt = parseInt(cnt)
        }

        // 0. if !columnCount && !columnWidth
        if (cnt === 'auto' && width === 'auto') {
          if (process.env.NODE_ENV === 'development') {
            console.warn(`[vue-render] waterfall's columnWidth and columnCount shouldn't`
            + ` both be auto at the same time.`)
            cnt = DEFAULT_COLUMN_COUNT
            width = ctWidth
          }
        }
        // 1. if columnCount = n then calc w.
        else if (cnt !== 'auto' && width === 'auto') {
          width = (ctWidth - (cnt - 1) * gap) / cnt
        }
        // 2. if columnWidth = w then calc n.
        else if (cnt === 'auto' && width !== 'auto') {
          cnt = (ctWidth + gap) / (width + gap)
        }
        // 3. if columnWidth = w and columnCount = n then calc totalWidth
        else if (cnt !== 'auto' && width !== 'auto') {
          let totalWidth
          const adjustCountAndWidth = () => {
            totalWidth = cnt * width + (cnt - 1) * gap
            if (totalWidth < ctWidth) {
              width += (ctWidth - totalWidth) / cnt
            }
            else if (totalWidth > ctWidth && cnt > 1) {
              cnt--
              adjustCountAndWidth()
            }
            else if (totalWidth > ctWidth) {  // cnt === 1
              width = ctWidth
            }
          }
          adjustCountAndWidth()
        }
        this._columnCount = cnt
        this._columnWidth = width
        this._columnGap = gap
      },

      _genColumns (createElement) {
        this._columns = []
        const cells = this._cells
        const columnCnt = this._columnCount
        const len = cells.length
        const columnCells = this._columnCells = Array(columnCnt).join('.').split('.').map(function () { return [] })
        // spread cells to the columns using simpole polling algorithm.
        for (let i = 0; i < len; i++) {
          (cells[i].data.attrs || (cells[i].data.attrs = {}))['data-cell'] = i
          columnCells[i % columnCnt].push(cells[i])
        }
        for (let i = 0; i < columnCnt; i++) {
          this._columns.push(createElement('html:div', {
            ref: `column${i}`,
            attrs: {
              'data-column': i
            },
            staticClass: 'weex-ct',
            staticStyle: {
              width: this._columnWidth + 'px',
              marginLeft: i === 0 ? 0 : this._columnGap + 'px'
            }
          }, columnCells[i]))
        }
      },

      _nextTick () {
        this._reLayoutChildren()
      },

      _reLayoutChildren () {
        /**
         * treat the shortest column bottom as the match standard.
         * whichever cell exceeded it would be rearranged.
         * 1. m = shortest column bottom.
         * 2. get all cell ids who is below m.
         * 3. calculate which cell should be in which column.
         */
        const columnCnt = this._columnCount
        const columnDoms = []
        const columnAppendFragments = []
        const columnBottoms = []
        let minBottom = Number.MAX_SAFE_INTEGER
        let minBottomColumnIndex = 0

        // 1. find the shortest column bottom.
        for (let i = 0; i < columnCnt; i++) {
          const columnDom = this._columns[i].elm
          const lastChild = columnDom.lastElementChild
          const bottom = lastChild ? lastChild.getBoundingClientRect().bottom : 0
          columnDoms.push(columnDom)
          columnBottoms[i] = bottom
          columnAppendFragments.push(document.createDocumentFragment())
          if (bottom < minBottom) {
            minBottom = bottom
            minBottomColumnIndex = i
          }
        }

        // 2. get all cell ids who is below m.
        const belowCellIds = []
        const belowCells = {}
        for (let i = 0; i < columnCnt; i++) {
          if (i === minBottomColumnIndex) {
            continue
          }
          const columnDom = columnDoms[i]
          const cellsInColumn = columnDom.querySelectorAll('section.weex-cell')
          const len = cellsInColumn.length
          for (let j = len - 1; j >= 0; j--) {
            const cellDom = cellsInColumn[j]
            const cellRect = cellDom.getBoundingClientRect()
            if (cellRect.top > minBottom) {
              const id = ~~cellDom.getAttribute('data-cell')
              belowCellIds.push(id)
              belowCells[id] = { elm: cellDom, height: cellRect.height }
              columnBottoms[i] -= cellRect.height
            }
          }
        }

        // 3. calculate which cell should be in which column and rearrange them
        //  in the dom tree.
        belowCellIds.sort(function (a, b) { return a > b })
        const cellIdsLen = belowCellIds.length
        function addToShortestColumn (belowCell) {
          // find shortest bottom.
          minBottom = Math.min(...columnBottoms)
          minBottomColumnIndex = columnBottoms.indexOf(minBottom)
          const { elm: cellElm, height: cellHeight } = belowCell
          columnAppendFragments[minBottomColumnIndex].appendChild(cellElm)
          columnBottoms[minBottomColumnIndex] += cellHeight
        }
        for (let i = 0; i < cellIdsLen; i++) {
          addToShortestColumn(belowCells[belowCellIds[i]])
        }
        for (let i = 0; i < columnCnt; i++) {
          columnDoms[i].appendChild(columnAppendFragments[i])
        }
      }
    },

    render (createElement) {
      this.weexType = 'waterfall'
      this._cells = this.$slots.default || []
      this.$nextTick(() => {
        this.updateLayout()
      })
      const mergedStyle = extractComponentStyle(this)
      return createElement('main', {
        ref: 'wrapper',
        attrs: { 'weex-type': 'waterfall' },
        on: createEventMap(this, {
          scroll: this.handleScroll,
          touchstart: this.handleTouchStart,
          touchmove: this.handleTouchMove,
          touchend: this.handleTouchEnd
        }),
        staticClass: 'weex-waterfall weex-waterfall-wrapper weex-ct',
        staticStyle: mergedStyle
      }, this._createChildren(createElement, mergedStyle))
    }
  }
}

export default {
  init (weex) {
    weex.registerComponent('waterfall', getWaterfall(weex))
  }
}
