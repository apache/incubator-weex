import { getHeadStyleMap } from '../../../../render/vue/core/style'
import '../data/head-css'
import headMap from '../data/head-map'

function toArray (arr) {
  return Array.prototype.slice.call(arr)
}

function getVStyleSheetNodes () {
  const regVStyleSheets = /((?:,?\s*\.[\w-]+\[data-v-\w+\](?::\w+)?)+)\s*({[^}]+)/
  const nodes = toArray(document.styleSheets)
    .filter(function (styleSheet) {
      return regVStyleSheets.test(styleSheet.ownerNode.textContent)
    }).map(function (styleSheet) {
      return styleSheet.ownerNode
    })
  return nodes
}

function getStyleSheetNodes () {
  return toArray(document.styleSheets)
    .map(function (styleSheet) {
      return styleSheet.ownerNode
    })
}

describe('style map', () => {
  let styleSheetNodes

  before(function () {
    styleSheetNodes = getStyleSheetNodes()
  })

  it('should get head style map.', () => {
    /**
     * before get style map.
     */
    const vStyleSheetNodes = getVStyleSheetNodes()
    const allNodes = getStyleSheetNodes()
    const vNodesExisted = vStyleSheetNodes.filter(function (node) {
      return allNodes.indexOf(node) > -1
    })
    expect(vNodesExisted).to.have.members(vStyleSheetNodes)

    /**
     * get style map.
     */
    window._no_remove_style_sheets = false
    const styleMap = getHeadStyleMap()
    window._no_remove_style_sheets = true

    /**
     * after get style map.
     */
    for (const key in headMap) {
      const expected = headMap[key]
      const result = styleMap[key]
      expect(expected).be.deep.equal(result)
    }

    const nowStyleSheetNodes = getStyleSheetNodes()
    const vNodesLeft = vStyleSheetNodes.filter(function (node) {
      return nowStyleSheetNodes.indexOf(node) > -1
    })
    expect(vNodesLeft).to.have.lengthOf(0)
  })

  after(function () {
    const nodes = getStyleSheetNodes()
    styleSheetNodes.forEach(function (node) {
      if (nodes.indexOf(node) === -1) {
        document.head.appendChild(node)
      }
    })
  })
})
