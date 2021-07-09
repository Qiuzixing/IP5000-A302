<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-model-title">Client IP Address</span>
        <span class="setting-model-title">Connection Protocol</span>
        <span class="setting-model-title">Client Port</span>
        <span class="setting-model-title">Device Port</span>
      </div>
      <div class="setting" v-for="(item, index) in list" :key="index">
        <span style="width: 200px;">${item[0]}</span>
        <span style="width: 200px;">${item[1]}</span>
        <span style="width: 200px;">${item[2]}</span>
        <span style="width: 200px;">${item[3]}</span>
      </div>
    </div>
  </div>
</template>

<script>

export default {
  name: 'status',
  data () {
    return {
      list: []
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#CONNECTION-LIST? ')
    this.$socket.sendMsg('#HW‑TEMP? 0')
    this.$socket.sendMsg('#PORT-DIRECTION? both.ir.1.ir')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@CONNECTION-LIST /i) !== -1) {
        this.handleConnList(msg)
      }
    },
    handleConnList (msg) {
      const startIndex = msg.indexOf('[')
      const result = msg.substr(startIndex).split(',')
      this.list = JSON.parse('[' + result.toString() + ']')
    }
  }
}
</script>
<style lang="less" scoped>
.setting-model-title,
.setting-title{
  width: 200px;
}
</style>
