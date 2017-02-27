import { extend } from './func'

const valueMap = {
  alignItems: {
    stretch: 'stretch',
    'flex-start': 'start',
    'flex-end': 'end',
    center: 'center'
  },

  flexDirection: {
    row: 'horizontal',
    column: 'vertical'
  },

  justifyContent: {
    'flex-start': 'start',
    'flex-end': 'end',
    center: 'center',
    'space-between': 'justify',
    'space-around': 'justify' // Just same as `space-between`
  }
}

const filters = {
  flex (value) {
    return {
      '-webkit-box-flex': value,
      '-webkit-flex': value,
      'flex': value
    }
  },

  'align-items' (value) {
    return {
      '-webkit-box-align': valueMap.alignItems[value],
      '-webkit-align-items': value,
      'align-items': value
    }
  },

  'align-self' (value) {
    return {
      '-webkit-align-self': value,
      'align-self': value
    }
  },

  'flex-direction' (value) {
    return {
      '-webkit-box-orient': valueMap.flexDirection[value],
      '-webkit-flex-direction': value,
      'flex-direction': value
    }
  },

  'justify-content' (value) {
    return {
      '-webkit-box-pack': valueMap.justifyContent[value],
      '-webkit-justify-content': value,
      'justify-content': value
    }
  }
}

export function normalize (styles) {
  const res = {}
  for (const key in styles) {
    if (typeof filters[key] !== 'function') {
      res[key] = styles[key]
    }
    else {
      extend(res, filters[key](styles[key]))
    }
  }
  return res
}
