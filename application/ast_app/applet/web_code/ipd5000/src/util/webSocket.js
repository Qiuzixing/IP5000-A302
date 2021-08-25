const socket = {
  ws: {},
  username: '',
  isLogin: false,
  // 收集发送和返回数据
  data: [],
  stopSend: false,
  host: window.location.protocol === 'http:' ? 'ws://' + window.location.host + '/websocket' : 'wss://' + window.location.host + '/websocket',
  // host: 'ws://' + window.location.hostname + ':19999',
  isCreateWs: false,
  supportWs: true,
  command: 'command',
  private: 'private',
  setting: 'setting',
  sendMsg (command, type) {
    if (this.ws && this.ws.readyState === 1) {
      command = command + '\r'
      this.ws.send(command)
      // if (!type) {
      //   console.log('Send:' + command)
      //   this.setData('Send : ' + command)
      // }
    }
  },
  initWebsocket () {
    if ('WebSocket' in window) {
      this.ws = new WebSocket(this.host)
      this.ws.onopen = this.open()
      this.isCreateWs = true
      this.supportWs = true
      this.ws.onclose = () => {
        this.closeWs()
      }
      return true
    } else if ('MozWebSocket' in window) {
      this.ws = new MozWebSocket(this.host)
      this.ws.onopen = this.open()
      this.isCreateWs = true
      this.supportWs = true
      this.ws.onclose = () => {
        this.closeWs()
      }
      return true
    } else {
      this.supportWs = false
      return false
    }
  },
  open () {
    console.log('Info: WebSocket connection opened.')
  },
  closeWs () {
    console.log('Info: WebSocket connection closed.')
    alert('Lost Connection!')
  }
}
export default socket
