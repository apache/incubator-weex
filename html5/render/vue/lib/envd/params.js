const lib = window.lib || (window.lib = {})
const env = lib.env || (lib.env = {})

const search = window.location.search.replace(/^\?/, '')
env.params = {}
if (search) {
  const params = search.split('&')
  for (let i = 0; i < params.length; i++) {
    params[i] = params[i].split('=')
    try {
      env.params[params[i][0]] = decodeURIComponent(params[i][1])
    }
    catch (e) {
      env.params[params[i][0]] = params[i][1]
    }
  }
}
