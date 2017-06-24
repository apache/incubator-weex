import websocket from '../../../../render/vue/modules/websocket/websocket'
describe('webSocket module', function () {
  afterEach(function (done) {
    // Cleanup
    websocket.close()
    done()
  })
  it('should have connection lifecycle constants defined on the instance', () => {
    expect(websocket.WebSocket('wss://echo.websocket.org').CONNECTING).to.be.equal(0)
  })
})
