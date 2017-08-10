import runtime from '../../../../runtime'
import frameworks from '../../../../frameworks'
import services from '../../../../services'

const defaultModules = {
  animation: ['transition'],
  clipboard: ['getString', 'setString'],
  dom: ['scrollToElement', 'getComponentRect', 'addRule'],
  modal: ['alert', 'confirm', 'prompt', 'toast'],
  navigator: ['push', 'pop'],
  picker: ['pick', 'pickDate', 'pickTime'],
  storage: ['setItem', 'getItem', 'removeItem', 'length', 'getAllKeys'],
  stream: ['fetch'],
  timer: ['setTimeout', 'setInterval'],
  WebSocket: ['send', 'close', 'onopen', 'onmessage', 'onclose', 'onerror'],
  webview: ['goBack', 'goForward', 'reload']
}

const defaultComponents = [
  'div', 'text', 'image', 'input', 'a', 'switch', 'scroller', 'list', 'cell',
  'slider', 'indicator', 'loading', 'refresh', 'textarea', 'web', 'video',
  'embed', 'richtext'
]

export function createContext () {
  const currentConfig = Object.assign({}, runtime.config)
  currentConfig.frameworks = frameworks

  for (const serviceName in services) {
    runtime.service.register(serviceName, services[serviceName])
  }

  const context = runtime.init(currentConfig)

  context.registerModules(defaultModules)
  context.registerComponents(defaultComponents)

  return context
}

// execute code and return the generated vdom tree
export function execute (type, code) {
  const context = createContext()
  return new Promise(resolve => {
    const id = String(Date.now() + Math.random())
    const instance = context.createInstance(id, `
      // { "framework": "${type}" }
      ${code}
    `)
    setTimeout(() => {
      resolve(instance.document.body.toJSON())
    }, 10)
  })
}
