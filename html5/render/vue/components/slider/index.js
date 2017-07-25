import slider from './slider'
import indicator from './indicator'

export default {
  init (weex) {
    weex.install(slider)
    weex.install(indicator)
  }
}
