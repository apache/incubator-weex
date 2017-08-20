import helper from '../main'

export const doneMixin = {
  methods: {
    done (id, ...args) {
      helper.done(id, ...args)
    }
  }
}
