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
             v-if="temperature < 60"></div>
        <div class="device-icon device-active"
             v-if="temperature >= 60 && temperature <= 70"></div>
        <div class="device-icon device-active"
             v-if="temperature > 70"></div>
        <span style="margin-left: 12px;">{{temperature}}&#8451; </span>
        <span style="padding: 0 5px">|</span>
        <span v-if="temperature < 60">Normal</span>
        <span v-if="temperature >= 60 && temperature <= 70">High</span>
        <span v-if="temperature > 70">Overheat</span>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Input status</h3>
        <div class="setting">
          <span class="setting-title"
                v-if="this.$global.deviceType">HDMI IN1</span>
          <span class="setting-title"
                v-else>HDMI IN</span>
          <div class="device-icon"
               :class="[hdmiin1 ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ hdmiin1 ? 'On' : 'Off'}}</span>
        </div>
        <div class="setting"
             v-if="this.$global.deviceType">
          <span class="setting-title">HDMI IN2</span>
          <div class="device-icon"
               :class="[hdmiin2 ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ hdmiin2 ? 'On' : 'Off'}}</span>
        </div>
        <div class="setting"
             v-if="this.$global.deviceType">
          <span class="setting-title">USB IN3</span>
          <div class="device-icon"
               :class="[usbin3 ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ usbin3 ? 'On' : 'Off'}}</span>
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
        <div class="setting">
          <span class="setting-title">LAN</span>
          <div class="device-icon"
               :class="[lan ? 'device-active' : 'device-default']"></div>
          <span style="margin-left: 12px;">{{ lan ? 'On' : 'Off'}}</span>
        </div>
      </div>
      <div class="setting-model">
        <h3 class="setting-model-title">Reset events log</h3>
        <div class="res-title">
          <span>Time</span>
          <span>Trigger</span>
        </div>
        <div class="res-info">
          <div class="res-info-item"
               v-for="(item, index) in log"
               :key="index">
            <span>{{item.date}} &nbsp;&nbsp; {{item.time}}</span>
            <span>{{item.type === '1' ? 'Auto' : 'Manual'}}</span>
          </div>
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
      temperature: 50,
      log: [],
      hdmiin1: false,
      hdmiin2: false,
      usbin3: false,
      hdmiout: false,
      lan: false
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#DEV-STATUS? ')
    this.$socket.sendMsg('#HW-TEMP? 0,0')
    this.$socket.sendMsg('#LOG-RESET? ')
    this.$socket.sendMsg('#SIGNALS-LIST? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@DEV-STATUS /i) !== -1) {
        this.handleDevStatus(msg)
        return
      }
      if (msg.search(/@HW‑TEMP /i) !== -1) {
        this.handleTemp(msg)
        return
      }
      if (msg.search(/@SIGNALS-LIST /i) !== -1) {
        this.handleSignal(msg)
        return
      }
      if (msg.search(/@LOG-RESET /i) !== -1) {
        this.handleLog(msg)
      }
    },
    handleDevStatus (msg) {
      this.deviceStatus = msg.split(' ')[1]
    },
    handleTemp (msg) {
      this.temperature = parseInt(msg.split(' ')[1].split(',')[1])
    },
    handleLog (msg) {
      const data = msg.split(' ')[1].split(',')
      this.log.push({
        type: data[0],
        date: data[1],
        time: data[2]
      })
    },
    handleSignal (msg) {
      this.hdmiin1 = msg.search(/in.hdmi.1/i) !== -1
      this.hdmiin2 = msg.search(/in.hdmi.2/i) !== -1
      this.usbin3 = msg.search(/in.usb/i) !== -1
      this.hdmiout = msg.search(/out.hdmi/i) !== -1
      this.lan = msg.search(/out.stream/i) !== -1
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
.res-title {
  display: flex;
  span {
    padding: 5px;
    font-family: "open sans semiblold";
  }
  span:first-child {
    width: 280px;
    background: #f3f3f3;
    margin-right: 24px;
  }
  span:last-child {
    width: 220px;
    background: #f3f3f3;
  }
}
.res-info {
  max-height: 280px;
  overflow-y: auto;
  .res-info-item {
    display: flex;
    span {
      width: 280px;
      word-wrap: break-word;
      padding: 5px;
    }
    span:first-child {
      margin-right: 24px;
    }
  }
}
</style>
