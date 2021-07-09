<template>
  <div class="main-setting">
    <div class="info">
      <div class="setting-model">
        <h3 class="info-title">Device Model</h3>
        <h3 class="info-desc">{{deviceModel}}</h3>
      </div>
      <div class="setting-model">
        <h3 class="info-title">Device HW Release</h3>
        <h3 class="info-desc">{{HWVer}}</h3>
      </div>
      <div class="setting-model">
        <h3 class="info-title">Firmware Version </h3>
        <h3 class="info-desc">{{ version }}</h3>
      </div>
      <div>
        <h3 class="info-company">Kramer Electronics Ltd.</h3>
        <div style="line-height: 1.5">
          3 Am VeOlamo St.<br>
          Jerusalem, Israel, 9546303<br>
          Tel: +972-73-2650200<br>
          Fax: +972-2-6535369<br>
          Email: info@kramerav.com<br>
          Web: KramerAV.com
        </div>
      </div>
    </div>
    <footer>
      &copy;&nbsp;&nbsp;2021 - Kramer Electronics Ltd. all rights reserved.
    </footer>
  </div>
</template>

<script>
export default {
  name: 'Info',
  data () {
    return {
      deviceModel: '',
      HWVer: '',
      version: ''
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#MODEL? ')
    this.$socket.sendMsg('#HW-VERSION? ')
    this.$socket.sendMsg('#VERSION? ')
  },
  methods: {
    handleMsg (msg) {
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
        return
      }
      if (msg.search(/@HW-VERSION /i) !== -1) {
        this.handleHWVersion(msg)
        return
      }
      if (msg.search(/@VERSION /i) !== -1) {
        this.handleVersion(msg)
      }
    },
    handleDeviceModel (msg) {
      this.deviceModel = msg.split(' ')[1]
    },
    handleHWVersion (msg) {
      this.HWVer = msg.split(' ')[1]
    },
    handleVersion (msg) {
      this.version = msg.split(' ')[1]
    }
  }
}
</script>

<style lang="less" scoped>
.info-title{
  font-family: 'open sans semiblold';
  font-size: 15px;
  margin-bottom: 13px;
  color: #35ACF8;
}
.info-desc{
  font-family: 'open sans bold';
  font-size: 15px;
  margin-bottom: 13px;
  margin-top: 13px;
}
.info-company{
  font-family: 'open sans semiblold';
  font-size: 15px;
  margin-bottom: 0;
}
.main-setting{
  display: flex;
  flex-direction: column;
  .info{
    flex: 1;
  }
  footer {
    flex-shrink: 0;
    margin-top: 15px;
    height: 30px;
    font-size: 13px;
  }
}
</style>
