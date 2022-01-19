<template>
  <div>
  </div>
</template>

<script>
export default {
  name: '',
  data () {
    return {
      count: 0
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.getModel()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
        return
      }
      if (msg.search(/@SECUR /i) !== -1) {
        this.handleSecurity(msg.trim().split(' ')[1])
      }
    },
    handleDeviceModel (msg) {
      const model = msg.split(' ')[1]
      this.$global.deviceModel = model
      this.$global.deviceType = this.modelType(model)
      document.title = model
      this.$socket.sendMsg('#SECUR? ')
    },
    getModel () {
      if (this.$socket.ws && this.$socket.ws.readyState === 1) {
        this.$socket.sendMsg('#MODEL? ')
      } else if (this.count < 20) {
        this.count++
        setTimeout(this.getModel, 300)
      }
    },
    modelType (model) {
      let type = 0
      switch (model) {
        case 'KDS-DEC7':
          type = 1
          break
        case 'WP-DEC7':
          type = 0
          break
        default:
          type = 0
          break
      }
      return type
    },
    handleSecurity (msg) {
      if (msg === '0') {
        this.$global.isLogin = true
        sessionStorage.setItem('login', 'true')
        this.$router.push('/main/av_routing')
      } else {
        this.$router.push('/login')
      }
    }
  }
}
</script>
