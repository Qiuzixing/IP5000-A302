<template>
  <div class="main-setting">
    <div class="setting-model">
      <h3 class="setting-model-title">General Preferences</h3>
      <div class="setting">
        <span class="setting-title">Host Name</span>
        <input class="setting-text" type="text" maxlength="63" v-model="hostname">
        <button type="button" class="btn btn-plain-primary" style="margin-left: 15px;" @click="setHostName">APPLY</button>
      </div>
      <div class="setting">
        <span class="setting-title">Device Model</span>
        <span>{{deviceModel}}</span>
<!--        <input type="text" class="setting-text" v-model="">-->
<!--        <button type="button" class="btn btn-plain-primary" style="margin-left: 15px;" @click="setDeviceModel">APPLY</button>-->
      </div>
      <div class="setting">
        <span class="setting-title">Device H/W Release</span>
        <span>{{HWVer}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">MAC Address</span>
        <span>{{macAddr}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Serial Number</span>
        <span>{{serialNum}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Front Panel Lock </span>
        <v-switch v-model="frontLock" active-value="1" inactive-value="0"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Import/Export Device Setting</span>
        <multiselect v-model="exportAndImport.val" :options="exportAndImport.param"></multiselect>
        <button type="button" class="btn btn-plain-primary" style="margin-left: 15px; margin-right: 15px;">IMPORT</button>
        <button type="button" class="btn btn-plain-primary">EXPORT</button>
      </div>
      <div class="setting">
        <span class="setting-title">Locate Device</span>
        <button type="button" class="btn btn-plain-primary" @click="locateDev">APPLY</button>
      </div>
      <div class="setting">
        <span class="setting-title">Power Save</span>
        <v-switch v-model="powerSave" active-value="1" inactive-value="0"></v-switch>
      </div>
      <div class="setting">
        <span class="setting-title">Inactivity Auto-standby Delay Duration</span>
        <el-input-number v-model="autoStandbyTime" controls-position="right" :max="30" :min="0"></el-input-number>
        <button class="btn btn-plain-primary" style="margin-left: 25px;" @click="setAutoStandbyTime">APPLY</button>
      </div>
    </div>
    <div class="setting-model">
      <h3 class="setting-model-title">Version</h3>
      <div class="setting">
        <span class="setting-title">Firmware Version</span>
        <span style="width: 180px;">{{version}}</span>
        <upload-components :show="false">UPGRADE</upload-components>
      </div>
      <div class="setting">
        <span class="setting-title">Last Upgrade Date/Time</span>
        <span>{{upgradeTime}}</span>
      </div>
      <div class="setting">
        <span class="setting-title">Firmware Standby Version  </span>
        <span>{{standbyVer}}</span>
        <button class="btn btn-plain-primary" @click="rollBack">ROLLBACK</button>
      </div>
    </div>
    <div class="setting-model">
      <div class="setting">
        <span class="setting-model-title">Device Reset</span>
        <button type="button" class="btn btn-plain-primary" style="margin-right: 70px" @click="dialogVisibleReset = true">RESTART</button>
        <button type="button" class="btn btn-plain-primary" @click="dialogVisibleFactory = true"><img src="../../assets/img/warning.svg" style="vertical-align: middle;margin-top: -5px" alt="">RESET</button>
      </div>
    </div>
    <el-dialog
        title="RESET"
        :visible.sync="dialogVisibleReset"
        width="500px"
        >
        <p class="dialog-second-title">Do you want to restart the device? </p>
        <span slot="footer" class="dialog-footer">
          <button class="btn btn-primary" @click="dialogVisibleReset = false, restart()">PROCEED</button>
          <button class="btn btn-primary" @click="dialogVisibleReset = false">CANCEL</button>
        </span>
    </el-dialog>
    <el-dialog
      title="RESET"
      :visible.sync="dialogVisibleFactory"
      width="500px"
    >
      <p class="dialog-second-title">Do you want to reset the device? </p>
      <span slot="footer" class="dialog-footer">
          <button class="btn btn-primary" @click="dialogVisibleFactory = false, reset()">PROCEED</button>
          <button class="btn btn-primary" @click="dialogVisibleFactory = false">CANCEL</button>
        </span>
    </el-dialog>
  </div>
</template>

<script>
import uploadComponents from '@/components/Upload.vue'
export default {
  name: 'autoSwitch',
  components: {
    uploadComponents
  },
  data () {
    return {
      show: false,
      upgrade: false,
      exportAndImport: {
        val: 'all',
        param: [
          { value: 'all', label: 'All' },
          { value: 'ip', label: 'Without IP' },
          { value: 'stream', label: 'Streams' },
          { value: 'av', label: 'AV Setting Only' }
        ]
      },
      displayOverlay: 'off',
      hostname: '',
      deviceModel: '',
      HWVer: '',
      macAddr: '',
      serialNum: '',
      delayDuration: '1',
      frontLock: '',
      powerSave: '',
      upgradeTime: '',
      standbyVer: '',
      version: '',
      dialogVisibleReset: false,
      dialogVisibleFactory: false,
      autoStandbyTime: 30
    }
  },
  beforeCreate () {
    this.$socket.ws.onmessage = msg => {
      this.handleMsg(msg.data.trim())
    }
  },
  created () {
    this.$socket.sendMsg('#NAME? ')
    this.$socket.sendMsg('#MODEL? ')
    this.$socket.sendMsg('#HW-VERSION? ')
    this.$socket.sendMsg('#NET-MAC? 0')
    this.$socket.sendMsg('#SN? ')
    this.$socket.sendMsg('#LOCK-FP? ')
    this.$socket.sendMsg('#STANDBY? ')
    this.$socket.sendMsg('#UPG-TIME? ')
    this.$socket.sendMsg('#VERSION? ')
    this.$socket.sendMsg('#STANDBY-VERSION? ')
  },
  methods: {
    handleMsg (msg) {
      console.log(msg)
      if (msg.search(/@NAME /i) !== -1) {
        this.handleHostname(msg)
        return
      }
      if (msg.search(/@MODEL /i) !== -1) {
        this.handleDeviceModel(msg)
        return
      }
      if (msg.search(/@HW-VERSION /i) !== -1) {
        this.handleHWVersion(msg)
        return
      }
      if (msg.search(/@NET-MAC /i) !== -1) {
        this.handleMACAddr(msg)
        return
      }
      if (msg.search(/@SN /i) !== -1) {
        this.handleSerialNum(msg)
        return
      }
      if (msg.search(/@LOCK-FP /i) !== -1) {
        this.handleFrontLock(msg)
        return
      }
      if (msg.search(/@STANDBY /i) !== -1) {
        this.handlePowerSave(msg)
        return
      }
      if (msg.search(/@UPG-TIME /i) !== -1) {
        this.handleUpgradeTime(msg)
        return
      }
      if (msg.search(/@STANDBY-VERSION /i) !== -1) {
        this.handleStandbyVer(msg)
        return
      }
      if (msg.search(/@KDS-STANDBY-TIMEOUT /i) !== -1) {
        this.handleAutoStandbyTime(msg)
        return
      }
      if (msg.search(/@VERSION /i) !== -1) {
        this.handleVersion(msg)
      }
    },
    handleHostname (msg) {
      this.hostname = msg.split(' ')[1]
    },
    handleDeviceModel (msg) {
      this.deviceModel = msg.split(' ')[1]
    },
    handleHWVersion (msg) {
      this.HWVer = msg.split(' ')[1]
    },
    handleMACAddr (msg) {
      this.macAddr = msg.split(' ')[1]
    },
    handleSerialNum (msg) {
      this.serialNum = msg.split(' ')[1]
    },
    handleFrontLock (msg) {
      this.frontLock = msg.split(' ')[1]
    },
    handlePowerSave (msg) {
      this.powerSave = msg.split(' ')[1]
    },
    handleUpgradeTime (msg) {
      this.upgradeTime = msg.split(' ')[1]
    },
    handleStandbyVer (msg) {
      this.standbyVer = msg.split(' ')[1]
    },
    handleVersion (msg) {
      this.version = msg.split(' ')[1]
    },
    restart () {
      this.$socket.sendMsg('#RESET')
    },
    reset () {
      this.$socket.sendMsg('#FACTORY')
    },
    setHostName () {
      this.$socket.sendMsg(`#NAME ${this.hostname}`)
    },
    setDeviceModel () {
      this.$socket.sendMsg(`#MODEL ${this.deviceModel}`)
    },
    locateDev () {
      this.$socket.sendMsg('#IDV')
    },
    setAutoStandbyTime () {
      this.$socket.sendMsg(`#KDS-STANDBY-TIMEOUT ${this.autoStandbyTime}`)
    },
    handleAutoStandbyTime (msg) {
      this.autoStandbyTime = parseInt(msg.split(' ')[1])
    },
    rollback () {
      this.$socket.sendMsg('#ROLLBACK')
    }
  }
}
</script>
<style lang="less" scoped>
//.upload-file{
//  display: none;
//}

</style>
