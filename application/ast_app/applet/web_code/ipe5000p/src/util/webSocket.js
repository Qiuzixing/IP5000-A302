const socket = {
  ws: {},
  username: '',
  isLogin: false,
  // 收集发送和返回数据
  data: [],
  stopSend: false,
  host: window.location.protocol === 'http:' ? 'ws://' + window.location.host + '/websocket' : 'wss://' + window.location.host + '/websocket',
  // host: 'ws://' + window.location.hostname + ':18888',
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
  setData (msg) {
    const _msg = msg.toLowerCase()
    if (_msg.startsWith('user') ||
      _msg.startsWith('login') ||
      _msg.startsWith('account')) {
      return
    }
    if (this.data.length >= 80) {
      this.data.pop()
    }
    this.data.unshift(this.getTime() + '   ' + msg)
  },
  getTime () {
    var thisDate = new Date()
    var thisTimeString = this.p0(thisDate.getHours()) + ':' + this.p0(thisDate.getMinutes()) + ':' + this.p0(thisDate.getSeconds())
    return thisTimeString
  },
  p0 (s) {
    return s < 10 ? '0' + s : s + ''
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
