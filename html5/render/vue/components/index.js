import _switch from './switch'
export { _switch as switch }

import a from './a'
import div from './div'
import image from './image'
import input from './input'
import header from './scrollable/header'
import list from './scrollable/list'
import cell from './scrollable/list/cell'
import scroller from './scrollable/scroller'
import slider from './slider'
import neighbor from './slider/slider-neighbor'
// import indicator from './warning'
import indicator from './slider/indicator'
// import refresh from './warning'
// import loading from './warning'
import refresh from './scrollable/refresh'
import loading from './scrollable/loading'
import LoadingIndicator from './scrollable/loading-indicator'
import text from './text'
import textarea from './textarea'
import video from './video'
import web from './web'

export default {
  a,
  div,
  container: div,
  image,
  img: image,
  input,
  header,
  list,
  cell,
  scroller,
  slider,
  'slider-neighbor': neighbor,
  indicator,
  refresh,
  loading,
  LoadingIndicator,
  text,
  textarea,
  video,
  web
}
