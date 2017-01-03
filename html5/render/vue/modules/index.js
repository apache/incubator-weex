import navigator from './navigator'

const modules = {
  navigator
}

export function requireModule (name) {
  if (modules[name]) {
    return modules[name]
  }
  console.log(`[Vue Render] Can't find "${name}" module.`)
  return null
}
