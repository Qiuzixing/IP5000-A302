const socket = {
  ws: {},
  username: '',
  isLogin: false,
  // 收集发送和返回数据
  data: [],
  stopSend: false,
  host: window.location.protocol === 'http:' ? 'ws://' + window.location.host + '/websocket' : 'wss://' + window.location.host + '/websocket',
  // host: 'ws://' + window.location.hostname + ':20000',
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
  callback: null,
  setCallback: function (callback) {
    this.callback = callback
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
      this.ws.onmessage = msg => {
        if (this.callback) {
          this.callback(msg.data.trim())
        }
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
    sessionStorage.removeItem('login')
    console.log('Info: WebSocket connection closed.')
    alert('Lost Connection!')
  }
}
export default socket
