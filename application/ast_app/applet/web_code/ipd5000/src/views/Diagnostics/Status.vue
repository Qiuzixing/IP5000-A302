<template>
  <div class="main-setting">
    <div class="setting-model">
      <div class="setting">
        <span class="setting-title">Device Status</span>
        <div class="device-icon device-active"
             v-if="deviceStatus === '0'"></div>
        <span v-if="deviceStatus === '0'"
              style="margin-left: 12px;">Active</span>
        <div class="device-icon device-default"
             v-if="deviceStatus === '1'"></div>
        <span v-if="deviceStatus === '1'"
              style="margin-left: 12px;">Standby</span>
      </div>
      <div class="setting">
        <span class="setting-title">Heat level</span>
        <div class="device-icon device-active"
             v-if="temperature <= 45"></div>
        <div class="device-icon device-warn"
             v-if="temperature > 45 && temperature <= 60"></div>
        <div class="device-icon device-error"
             v-if="temperature > 60"></div>
        <span style="margin-left: 12px;">{{temperature}} &#8451;</span>
        <span style="padding: 0 5px">|</span>
        <span v-if="temperature <= 45">Normal</span>
        <span v-if="temperature > 45 && temperature <= 60">High</span>
        <span v-if="temperature > 60">Overheat</span>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Input status</h3>
        <div class="setting"
             v-if="this.$global.deviceType">
          <span class="setting-title">HDMI IN</span>
          <div class="device-icon"
               :class="[hdmiin1 ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ hdmiin1 ? 'On' : 'Off'}}</span>
        </div>
        <div class="setting">
          <span class="setting-title">LAN</span>
          <div class="device-icon"
               :class="[hdmiin2 ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ hdmiin2 ? 'On' : 'Off'}}</span>
        </div>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Output status</h3>
        <div class="setting">
          <span class="setting-title">HDMI OUT</span>
          <div class="device-icon"
               :class="[hdmiout ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ hdmiout ? 'On' : 'Off'}}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script>

export default {
  name: 'status',
  data () {
    return {
      deviceStatus: '1',
      temperature: 0,
      hdmiin1: false,
      hdmiin2: false,
      usbin3: false,
      hdmiout: false,
      lan: false
    }
  },
  created () {
    this.$socket.setCallback(this.handleMsg)
    this.$socket.sendMsg('#DEV-STATUS? ')
    this.$socket.sendMsg('#HW-TEMP? 0,0')
    this.$socket.sendMsg('#SIGNALS-LIST? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@DEV-STATUS /i) !== -1) {
        this.handleDevStatus(msg)
        return
      }
      if (msg.search(/@HW-TEMP /i) !== -1) {
        this.handleTemp(msg)
        return
      }
      if (msg.search(/@SIGNALS-LIST /i) !== -1) {
        this.handleSignal(msg)
      }
    },
    handleDevStatus (msg) {
      this.deviceStatus = msg.split(' ')[1]
    },
    handleTemp (msg) {
      const data = msg.split(' ')[1].split(',')
      if (data[0] === '0') {
        this.temperature = parseInt(data[1])
      }
    },
    handleSignal (msg) {
      this.hdmiin1 = msg.search(/in.hdmi.1/i) !== -1
      this.hdmiin2 = msg.search(/in.stream.1/i) !== -1
      this.hdmiout = msg.search(/out.hdmi/i) !== -1
    }
  }
}
</script>
<style lang="less" scoped>
.setting-title {
  width: 150px;
}
.device-icon {
  width: 28px;
  height: 28px;
}
.device-active {
  background: #41bd64;
  width: 16px;
  height: 16px;
  border-radius: 100%;
}
.device-warn {
  background: #ffca28;
  width: 16px;
  height: 16px;
  border-radius: 100%;
}
.device-error {
  background: #d50000;
  width: 16px;
  height: 16px;
  border-radius: 100%;
}
.device-default {
  background: #d0d0d0;
  width: 16px;
  height: 16px;
  border-radius: 100%;
}

</style>
