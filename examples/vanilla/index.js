// { "framework": "Vanilla" }

var body = weex.document.createElement('div', {
  classStyle: { alignItems: 'center', marginTop: 120 }
})

var image = weex.document.createElement('image', {
  attr: { src: 'https://alibaba.github.io/weex/img/weex_logo_blue@3x.png' },
  classStyle: { width: 360, height: 82 }
})

var text = weex.document.createElement('text', {
  attr: { value: 'Hello World' },
  classStyle: { fontSize: 48 }
})

body.appendChild(image)
body.appendChild(text)
weex.document.documentElement.appendChild(body)

body.addEvent('click', function () {
  text.setAttr('value', 'Hello Weex')
})
