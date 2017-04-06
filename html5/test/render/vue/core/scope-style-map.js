import {
  getHeadStyleMap
} from '../../../../render/vue/core/style'
import '../data/head-css'
import headMap from '../data/head-map'

describe('style map', (Vue, helper) => {
  it('should get head style map.', () => {
    const styleMap = getHeadStyleMap()
    for (const key in headMap) {
      const expected = headMap[key]
      const result = styleMap[key]
      expect(expected).be.deep.equal(result)
    }
  })
})
