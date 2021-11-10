<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-model-title">Connection Protocol</span>
        <span class="setting-model-title">Client IP Address</span>

        <span class="setting-model-title">Client Port</span>
        <span class="setting-model-title">Device Port</span>
      </div>
      <div class="setting"
           v-for="(item, index) in list"
           :key="index">
        <span style="width: 200px;">{{item[0]}}</span>
        <span style="width: 200px;">{{item[1]}}</span>
        <span style="width: 200px;">{{item[2]}}</span>
        <span style="width: 200px;">{{item[3]}}</span>
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
    this.$socket.sendMsg('#NET-STAT? ')
    this.$socket.sendMsg('#HW‑TEMP? 0')
    this.$socket.sendMsg('#PORT-DIRECTION? both.ir.1.ir')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@NET-STAT /i) !== -1) {
        this.handleConnList(msg)
      }
    },
    handleConnList (msg) {
      const data = msg.match(/\w*:\d+,\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3}:\d+/g)
      const list = []
      for (const i in data) {
        const regx = /^(\w*):(\d+),(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3}):(\d+)$/
        regx.test(data[i])
        list.push([RegExp.$1, RegExp.$3, RegExp.$4, RegExp.$2])
      }
      this.list = list
    }
  }
}
</script>
<style lang="less" scoped>
.setting-model-title,
.setting-title {
  width: 200px;
}
</style>
