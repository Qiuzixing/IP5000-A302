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
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.getModel()
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
      }
    },
    handleDeviceModel (msg) {
      const model = msg.split(' ')[1]
      this.$global.deviceModel = model
      this.$global.deviceType = this.modelType(model)
      document.title = model
      this.$global.isLogin = true
      this.$router.push(this.$route.query.redirect)
    },
    getModel () {
      if (this.$socket.ws && this.$socket.ws.readyState === 1) {
        this.$socket.sendMsg('#MODEL? ')
      } else if (this.count < 20) {
        this.count++
        setTimeout(this.getModel, 100)
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
    }
  }
}
</script>
